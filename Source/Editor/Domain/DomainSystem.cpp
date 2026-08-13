#include "DomainSystem.h"

#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

#include <unordered_set>
#include <utility>

namespace Horizon::Editor
{
	DomainSystem::DomainSystem(const std::string& projectPath) : m_projectPath(projectPath), m_clock(PAL::Timer())
	{
	}

	DomainSystem::~DomainSystem()
	{

	}

	Engine::AppReport DomainSystem::OnAttach(Engine::Application* pEngine)
	{
		m_assetPath = m_projectPath + "/Assets";

		// Check if this is something we have
		if (!PAL::Directory::Exists(m_assetPath))
			PAL::Directory::Create(m_assetPath);

		// Clock starts ticking motherfucker.
		m_clock.Start();

		m_watcher = PAL::DirectoryWatcher(m_assetPath, true);
		m_watcherHealthy = m_watcher.IsValid();

		if (!m_watcherHealthy)
			Terminal::Warn("DomainSystem", "{} cannot be watched, falling back to periodic scanning", m_assetPath);

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath);
		RunFullRescan(m_clock.GetElapsedTimeInSec());
		return Engine::AppReport();
	}

	void DomainSystem::OnSync()
	{
		f64 now = m_clock.GetElapsedTimeInSec();

		if (m_watcherHealthy)
			DrainWatcher(now);

		f64 scanInterval = m_watcherHealthy ? SafetyScanSeconds : FallbackScanSeconds;
		if (now - m_lastFullScan >= scanInterval)
			m_fullRescan = true;

		if (m_fullRescan)
		{
			RunFullRescan(now);
			return;
		}

		ProcessDirtyFolders(now);
		ProcessPendingImports(now);
	}

	void DomainSystem::OnDetach()
	{
		m_watcher = PAL::DirectoryWatcher();
		m_watcherHealthy = false;

		m_events.Clear();
		m_dirtyFolders.clear();
		m_pendingImports.clear();

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

	void DomainSystem::DrainWatcher(f64 now)
	{
		m_events.Clear();

		b8 healthy = m_watcher.Poll(m_events);

		for (const PAL::DirectoryWatcher::Event& event : m_events)
		{
			if (event.action == PAL::WatcherAction::Overflow)
			{
				m_fullRescan = true;
				m_dirtyFolders.clear();
				break;
			}

			if (event.action == PAL::WatcherAction::Modified && !event.isDirectory)
			{
				MarkImport(event.path, now);
				continue;
			}

			MarkDirty(StringOps::ParentPathOf(event.path), now);

			if (event.action == PAL::WatcherAction::Renamed)
				MarkDirty(StringOps::ParentPathOf(event.oldPath), now);
		}

		if (healthy)
			return;

		Terminal::Warn("DomainSystem", "{} watcher stopped, falling back to periodic scanning", m_assetPath);

		m_watcherHealthy = false;
		m_fullRescan = true;
	}

	void DomainSystem::ProcessDirtyFolders(f64 now)
	{
		usize processed = 0;

		for (auto it = m_dirtyFolders.begin(); it != m_dirtyFolders.end() && processed < MaxFolderSyncPerFrame; )
		{
			if (now - it->second < DebounceSeconds)
			{
				++it;
				continue;
			}

			b8 exact = false;
			DomainFolder* pFolder = ResolveFolder(it->first, exact);

			if (pFolder != nullptr)
				SyncFolder(pFolder, !exact);

			it = m_dirtyFolders.erase(it);
			++processed;
		}
	}

	void DomainSystem::ProcessPendingImports(f64 now)
	{
		usize processed = 0;

		for (auto it = m_pendingImports.begin(); it != m_pendingImports.end() && processed < MaxImportsPerFrame; )
		{
			if (now - it->second < DebounceSeconds)
			{
				++it;
				continue;
			}

			if (!PAL::File::Exists(it->first))
			{
				it = m_pendingImports.erase(it);
				continue;
			}

			// TODO: Re-import / re-cook the source file here. The payload must stay a plain
			// path so this can move onto JobContext once the job lanes exist, with the
			// result applied back on the main thread.

			it = m_pendingImports.erase(it);
			++processed;
		}
	}

	void DomainSystem::RunFullRescan(f64 now)
	{
		m_fullRescan = false;
		m_lastFullScan = now;
		m_dirtyFolders.clear();

		SyncFolder(m_root, true);
	}

	void DomainSystem::SyncFolder(DomainFolder* pTarget, b8 recursive)
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

				SyncFolder(pChild, true);
				continue;
			}

			if (recursive)
				SyncFolder(pChild, true);
		}

		pTarget->m_folders.Sort([](const DomainFolder* pA, const DomainFolder* pB)
			{
				return pA->GetName() < pB->GetName();
			});
	}

	void DomainSystem::MarkDirty(const std::string& absolutePath, f64 now)
	{
		if (absolutePath.empty())
			return;

		m_dirtyFolders[absolutePath] = now;
	}

	void DomainSystem::MarkImport(const std::string& absolutePath, f64 now)
	{
		if (absolutePath.empty() || absolutePath.ends_with(MetaFileExt))
			return;

		m_pendingImports[absolutePath] = now;
	}

	DomainFolder* DomainSystem::ResolveFolder(const std::string& absolutePath, b8& outExact) const
	{
		outExact = false;

		if (m_root == nullptr)
			return nullptr;

		if (!StringOps::StartsWithNoCase(absolutePath, m_assetPath))
		{
			Terminal::Debug("DomainSystem", "{} is outside of the watched asset root", absolutePath);
			return nullptr;
		}

		DomainFolder* pCurrent = m_root;
		usize cursor = m_assetPath.size();

		while (cursor < absolutePath.size())
		{
			while (cursor < absolutePath.size() && StringOps::IsSeparator(absolutePath[cursor]))
				++cursor;

			usize begin = cursor;

			while (cursor < absolutePath.size() && !StringOps::IsSeparator(absolutePath[cursor]))
				++cursor;

			if (begin == cursor)
				break;

			DomainFolder* pChild = FindFolder(pCurrent->GetFolders(), absolutePath.substr(begin, cursor - begin));

			if (pChild == nullptr)
				return pCurrent;

			pCurrent = pChild;
		}

		outExact = true;
		return pCurrent;
	}

	DomainFolder* DomainSystem::FindFolder(const List<DomainFolder*>& folders, const std::string& folderName) const
	{
		for (DomainFolder* pFolder : folders)
		{
			if (StringOps::EqualsNoCase(pFolder->GetName(), folderName))
				return pFolder;
		}

		Terminal::Debug("DomainSystem", "{} folder is not tracked yet", folderName);
		return nullptr;
	}
}