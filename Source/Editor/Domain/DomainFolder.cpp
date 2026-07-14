#include "DomainFolder.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSystem.h>

#include <Editor/Domain/DomainFile.h>

namespace Horizon
{
	DomainFolder::DomainFolder(const DomainFolderDesc& desc, Engine* pEngine) : m_relativePath(desc.relativePath),
		m_parent(desc.pParent), m_engine(pEngine), m_absolutePath(desc.absolutePath), m_name(desc.folderName)
	{
		auto* pAssetSub = m_engine->TryGetSystem<AssetSystem>();
		if (!pAssetSub)
		{
			Terminal::Error("DomainFile", "Could not catch asset System");
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(m_absolutePath))
		{
			if (entry.is_directory())
			{
				DomainFolderDesc desc = {};
				desc.absolutePath = entry.path();
				desc.relativePath = m_relativePath / entry.path().filename();
				desc.folderName = entry.path().filename().string();
				desc.pParent = this;

				auto* newFolder = Allocator::Create<DomainFolder>(CurrLoc(), desc, pEngine);
				m_subFolders.push_back(newFolder);
				continue;
			}

			if (entry.is_regular_file())
			{
				// Check if its other than hmeta
				if (entry.path().extension() != ".hmeta")
				{
					Terminal::Warn("DomainFolder", "{} folder has unsupported file {}", m_relativePath, entry.path().string());
					continue;
				}

				// Read meta file

			}
		}
	}

	DomainFolder::~DomainFolder()
	{
		for (auto* file : m_files)
			Allocator::Delete(file);
		m_files.clear();

		for (auto* folder : m_subFolders)
			Allocator::Delete(folder);
		m_subFolders.clear();
	}

	b8 DomainFolder::HasFile(const std::string& fileName)
	{
		for (auto* file : m_files)
		{
			if (file->GetName() == fileName)
				return true;
		}

		return false;
	}

	b8 DomainFolder::HasFolder(const std::string& folderName)
	{
		for (auto* folder : m_subFolders)
		{
			if (folder->GetName() == folderName)
				return true;
		}

		return false;
	}

	DomainFile* DomainFolder::FindFile(const std::string& name)
	{
		for (auto* file : m_files)
		{
			if (file->GetName() == name)
				return file;
		}

		return nullptr;
	}

	DomainFolder* DomainFolder::FindFolder(const std::string& name)
	{
		for (auto* folder : m_subFolders)
		{
			if (folder->GetName() == name)
				return folder;
		}

		return nullptr;
	}

	void DomainFolder::ResetChildMarks()
	{
		for (auto* file : m_files)
			file->Unmark();

		for (auto* folder : m_subFolders)
			folder->Unmark();
	}

	void DomainFolder::SweepUnmarked()
	{
		std::erase_if(m_files, [](DomainFile* f)
			{
				if (!f->IsMarked()) { Allocator::Delete(f); return true; }
				return false;
			});

		std::erase_if(m_subFolders, [](DomainFolder* d)
			{
				if (!d->IsMarked()) { Allocator::Delete(d); return true; }
				return false;
			});
	}
}