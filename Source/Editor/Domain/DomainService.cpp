#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/ModuleGraph.h>
#include <Engine/Asset/AssetService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	namespace
	{
		std::string ChildPath(std::string_view parentPath, std::string_view name)
		{
			if (parentPath.empty())
				return std::string(name);

			return std::string(parentPath) + "/" + std::string(name);
		}

		DomainFolder* FindChild(const List<DomainFolder*>& folders, const std::string& name)
		{
			for (DomainFolder* pFolder : folders)
			{
				if (StringOps::EqualsNoCase(pFolder->GetName(), name))
					return pFolder;
			}

			return nullptr;
		}

		b8 SnapshotHasFolder(const DomainFolderSnapshot& snapshot, const std::string& name)
		{
			for (const DomainEntrySnapshot& entry : snapshot.entries)
			{
				if (entry.isDirectory && StringOps::EqualsNoCase(entry.name, name))
					return true;
			}

			return false;
		}
	}

	DomainService::DomainService()
	{
	}

	DomainService::~DomainService()
	{
	}

	Engine::ModuleReport DomainService::OnInitialize()
	{
		m_projectPath = "D:/Projects/Horizon/ExampleProject";
		m_assetPath = m_projectPath + "/Assets";

		if (!PAL::Directory::Exists(m_assetPath) && !PAL::Directory::Create(m_assetPath))
			return Engine::ModuleReport("Asset root cannot be created");

		m_clock.Start();

		m_watcher = PAL::DirectoryWatcher(m_assetPath, true);
		m_watcherHealthy = m_watcher.IsValid();

		if (!m_watcherHealthy)
			Terminal::Warn(StringOps::GetName(this), "{} cannot be watched, falling back to periodic scanning", m_assetPath);

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath);
		m_folderIndex.emplace(std::string(), m_root);

		return Engine::ModuleReport();
	}

	void DomainService::OnExecute()
	{
		DrainResults();
		SubmitScan();
	}

	void DomainService::OnFinalize()
	{
		m_watcher = PAL::DirectoryWatcher();
		m_watcherHealthy = false;

		m_events.Clear();
		m_dirtyFolders.clear();
		m_pendingResults.Clear();
		m_folderIndex.clear();

		if (m_root == nullptr)
			return;

		Memory::Allocator::Delete(m_root);
		m_root = nullptr;
	}

	void DomainService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Precedes<Engine::AssetService>();
	}

	DomainFolder* DomainService::FindFolder(std::string_view relativePath) const
	{
		auto it = m_folderIndex.find(relativePath);

		if (it == m_folderIndex.end())
		{
			Terminal::Debug("DomainService", "{} folder is not tracked", relativePath);
			return nullptr;
		}

		return it->second;
	}

	void DomainService::SubmitScan()
	{
		if (m_scanBusy.CompareExchange(0, 1) != 0)
			return;

		RunScan();
	}

	void DomainService::DrainResults()
	{
		List<DomainScanResult> results;

		m_resultLock.Lock();
		results.Swap(m_pendingResults);
		m_resultLock.Unlock();

		if (results.IsEmpty())
			return;

		for (const DomainScanResult& result : results)
		{
			for (const DomainFolderSnapshot& snapshot : result.folders)
				ApplyFolderSnapshot(snapshot);
		}

		++m_revision;
	}

	void DomainService::ApplyFolderSnapshot(const DomainFolderSnapshot& snapshot)
	{
		DomainFolder* pFolder = FindFolder(snapshot.relativePath);

		if (pFolder == nullptr)
			return;

		for (DomainFile* pFile : pFolder->m_files)
			Memory::Allocator::Delete(pFile);

		pFolder->m_files.Clear();

		for (usize i = pFolder->m_folders.GetCount(); i > 0; --i)
		{
			DomainFolder* pChild = pFolder->m_folders[i - 1];

			if (SnapshotHasFolder(snapshot, pChild->GetName()))
				continue;

			DestroyFolder(pChild, ChildPath(snapshot.relativePath, pChild->GetName()));
			pFolder->m_folders.RemoveAt(i - 1);
		}

		for (const DomainEntrySnapshot& entry : snapshot.entries)
		{
			if (!entry.isDirectory)
			{
				pFolder->m_files.PushBack(Memory::Allocator::Create<DomainFile>(Memory::CurrLoc(),
					entry.id, pFolder, entry.name, entry.metaPath, entry.absolutePath));
				continue;
			}

			if (FindChild(pFolder->m_folders, entry.name) != nullptr)
				continue;

			DomainFolder* pChild = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(),
				pFolder, entry.name, entry.absolutePath);

			pFolder->m_folders.PushBack(pChild);
			m_folderIndex.emplace(ChildPath(snapshot.relativePath, entry.name), pChild);
		}

		pFolder->m_folders.Sort([](const DomainFolder* pLeft, const DomainFolder* pRight)
			{
				return pLeft->GetName() < pRight->GetName();
			});
	}

	void DomainService::DestroyFolder(DomainFolder* pFolder, const std::string& relativePath)
	{
		for (DomainFolder* pChild : pFolder->m_folders)
			DestroyFolder(pChild, ChildPath(relativePath, pChild->GetName()));

		pFolder->m_folders.Clear();

		m_folderIndex.erase(relativePath);
		Memory::Allocator::Delete(pFolder);
	}

	void DomainService::RunScan()
	{
		const f64 now = m_clock.GetElapsedTimeInSec();

		if (m_rescanRequest.Exchange(0) != 0)
			m_pendingFullRescan = true;

		if (m_watcherHealthy)
			CollectDirty(now);

		const f64 scanInterval = m_watcherHealthy ? SafetyScanSeconds : FallbackScanSeconds;

		if (now - m_lastFullScan >= scanInterval)
			m_pendingFullRescan = true;

		DomainScanResult result;

		if (m_pendingFullRescan)
		{
			m_pendingFullRescan = false;
			m_lastFullScan = now;
			m_dirtyFolders.clear();

			result.fullRescan = true;
			ScanFolder(std::string(), true, result);
		}
		else
		{
			for (auto it = m_dirtyFolders.begin(); it != m_dirtyFolders.end(); )
			{
				if (now - it->second.timestamp < DebounceSeconds)
				{
					++it;
					continue;
				}

				ScanFolder(it->first, it->second.recursive, result);
				it = m_dirtyFolders.erase(it);
			}
		}

		if (!result.folders.IsEmpty())
		{
			m_resultLock.Lock();
			m_pendingResults.PushBack(std::move(result));
			m_resultLock.Unlock();
		}

		m_scanBusy.Store(0);
	}

	void DomainService::CollectDirty(f64 now)
	{
		m_events.Clear();

		const b8 healthy = m_watcher.Poll(m_events);

		for (const PAL::DirectoryWatcher::Event& event : m_events)
		{
			if (event.action == PAL::WatcherAction::Overflow)
			{
				m_pendingFullRescan = true;
				m_dirtyFolders.clear();
				break;
			}

			if (event.GetExtension() == MetaExtension)
				continue;

			if (event.action == PAL::WatcherAction::Modified)
			{
				continue;
			}

			if (event.action == PAL::WatcherAction::Renamed)
				MarkDirty(event.GetOldParent(), now, false);

			if (event.kind == PAL::WatcherEntryKind::Directory)
				MarkDirty(event.relativePath, now, true);

			MarkDirty(event.GetParent(), now, false);
		}

		if (healthy)
			return;

		Terminal::Warn(StringOps::GetName(this), "{} watcher stopped, falling back to periodic scanning", m_assetPath);

		m_watcherHealthy = false;
		m_pendingFullRescan = true;
	}

	void DomainService::MarkDirty(std::string_view relativePath, f64 now, b8 recursive)
	{
		DirtyFolder& dirty = m_dirtyFolders[std::string(relativePath)];

		dirty.timestamp = now;
		dirty.recursive = dirty.recursive || recursive;
	}

	void DomainService::ScanFolder(const std::string& relativePath, b8 recursive, DomainScanResult& outResult)
	{
		const std::string absolutePath = ToAbsolute(relativePath);
		const List<PAL::Directory::Entry> diskEntries = PAL::Directory::Iterate(absolutePath);

		DomainFolderSnapshot snapshot;
		snapshot.relativePath = relativePath;

		for (const PAL::Directory::Entry& diskEntry : diskEntries)
		{
			if (!diskEntry.isDirectory && diskEntry.name.ends_with(MetaSuffix))
				continue;

			DomainEntrySnapshot entry;
			entry.name = diskEntry.name;
			entry.absolutePath = diskEntry.fullPath;
			entry.isDirectory = diskEntry.isDirectory;

			if (!diskEntry.isDirectory)
			{
				entry.metaPath = diskEntry.fullPath + std::string(MetaSuffix);
				entry.assetTypeName = "Unknown";

			}

			snapshot.entries.PushBack(std::move(entry));
		}

		snapshot.entries.Sort([](const DomainEntrySnapshot& left, const DomainEntrySnapshot& right)
			{
				if (left.isDirectory != right.isDirectory)
					return left.isDirectory;

				return left.name < right.name;
			});

		outResult.folders.PushBack(std::move(snapshot));

		if (!recursive)
			return;

		for (const PAL::Directory::Entry& diskEntry : diskEntries)
		{
			if (!diskEntry.isDirectory)
				continue;

			ScanFolder(ChildPath(relativePath, diskEntry.name), true, outResult);
		}
	}

	std::string DomainService::ToAbsolute(std::string_view relativePath) const
	{
		if (relativePath.empty())
			return m_assetPath;

		return m_assetPath + "/" + std::string(relativePath);
	}
}