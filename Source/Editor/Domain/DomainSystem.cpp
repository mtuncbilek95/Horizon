#include "DomainSystem.h"

#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

#include <unordered_set>

namespace Horizon::Editor
{
	DomainSystem::DomainSystem(const std::string& projectPath) : m_projectPath(projectPath)
	{
	}

	DomainSystem::~DomainSystem()
	{

	}

	Engine::AppReport DomainSystem::OnAttach(Engine::Application* pEngine)
	{
		std::string assetPath = m_projectPath + "/Assets";

		// Check if the path exists
		if (!PAL::Directory::Exists(assetPath))
			PAL::Directory::Create(assetPath);

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", assetPath);
		
		UpdateFolder(m_root);
		m_lastSpan.Start();

		return Engine::AppReport();
	}

	void DomainSystem::OnSync()
	{
		if (m_lastSpan.GetElapsedTimeInSec() < ScanIntervalSeconds)
			return;
		m_lastSpan.Reset();

		UpdateFolder(m_root);
	}

	void DomainSystem::OnDetach()
	{
		if (m_root == nullptr)
			return;

		Memory::Allocator::Delete(m_root);
		m_root = nullptr;
	}

	void DomainSystem::GetInitializeOrder(Engine::OrderRules& rules) const
	{
		Requires<EditorSystem>(rules.before); // TODO: Add AssetSystem later
	}

	void DomainSystem::GetExecutionOrder(Engine::OrderRules& rules) const
	{
		Requires<EditorSystem>(rules.before); // TODO: Add AssetSystem later
	}

	void DomainSystem::UpdateFolder(DomainFolder* pTarget)
	{
		if (pTarget == nullptr)
			return;

		List<PAL::Directory::Entry> entries = PAL::Directory::Iterate(pTarget->m_absolutePath);

		std::unordered_set<std::string> filesOnDisk;
		std::unordered_set<std::string> foldersOnDisk;

		for (const PAL::Directory::Entry& entry : entries)
		{
			if (entry.isDirectory)
				foldersOnDisk.insert(entry.name);
			else
				filesOnDisk.insert(entry.fullPath);
		}

		// Check for folders
		for (const PAL::Directory::Entry& entry : entries)
		{
			if (entry.isDirectory || !entry.name.ends_with(MetaFileExt))
				continue;

			std::string sourcePath = entry.fullPath.substr(0, entry.fullPath.size() - MetaFileExt.size());

			if (filesOnDisk.contains(sourcePath))
				continue;

			if (!PAL::File::Delete(entry.fullPath))
			{
				Terminal::Error("DomainSystem", "Cannot delete orphan meta: {}", entry.fullPath);
				continue;
			}

			filesOnDisk.erase(entry.fullPath);
			Terminal::Warn("DomainSystem", "Orphan meta removed: {}", entry.fullPath);
		}

		for (DomainFile* pFile : pTarget->m_files)
			Memory::Allocator::Delete(pFile);

		pTarget->m_files.Clear();

		// Check for weird extension files such as .fbx, .gltf, .png etc
		for (const PAL::Directory::Entry& entry : entries)
		{
			if (entry.isDirectory || entry.name.ends_with(MetaFileExt))
				continue;

			std::string metaPath = entry.fullPath + std::string(MetaFileExt);

			// TODO: Guid guid; if (!TryReadOrCreateMetaFile(metaPath, guid)) continue;
			// pTarget->m_files.PushBack(Memory::Allocator::Create<DomainFile>(CurrLoc(), guid, pTarget, entry.name, metaPath, entry.fullPath));
		}

		pTarget->m_files.Sort([](const DomainFile* pA, const DomainFile* pB)
			{
				return pA->GetName() < pB->GetName();
			});

		for (usize i = pTarget->m_folders.GetCount(); i > 0; --i)
		{
			DomainFolder* pFolder = pTarget->m_folders[i - 1];

			if (foldersOnDisk.contains(pFolder->GetName()))
				continue;

			Memory::Allocator::Delete(pFolder);
			pTarget->m_folders.RemoveAt(i - 1);
		}

		for (const PAL::Directory::Entry& entry : entries)
		{
			if (!entry.isDirectory)
				continue;

			DomainFolder* pChild = FindFolder(pTarget->m_folders, entry.name);

			if (pChild == nullptr)
			{
				pChild = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), pTarget, entry.name, entry.fullPath);
				pTarget->m_folders.PushBack(pChild);
			}

			UpdateFolder(pChild);
		}

		pTarget->m_folders.Sort([](const DomainFolder* pA, const DomainFolder* pB)
			{
				return pA->GetName() < pB->GetName();
			});
	}

	DomainFolder* DomainSystem::FindFolder(const List<DomainFolder*>& folders, const std::string& folderName)
	{
		for (DomainFolder* pFolder : folders)
		{
			if (pFolder->GetName() == folderName)
				return pFolder;
		}

		Terminal::Debug("DomainSystem", "{} folder is not tracked yet", folderName);
		return nullptr;
	}
}