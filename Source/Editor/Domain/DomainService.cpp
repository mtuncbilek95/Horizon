#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

#include <utility>

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
		// TODO(Project): this must come from the project context / argv once it exists.
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
		Engine::JobSystem* pJobSystem = GetEngine()->GetJobSystem();

		if (pJobSystem != nullptr)
			pJobSystem->Wait(m_scanCounter);

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

		Engine::JobSystem* pJobSystem = GetEngine()->GetJobSystem();

		if (pJobSystem == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Job system is unavailable, the domain cannot be scanned");
			m_scanBusy.Store(0);
			return;
		}

		pJobSystem->Dispatch(Engine::JobLane::Background, m_scanCounter, [this]()
			{
				RunScan();
			});
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
				// TODO(Cook): a modified source file has to be queued for re import here.
				// Its folder listing did not change, so no snapshot is needed for it.
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

				// TODO(Meta): generate meta here. Read <source>.hmeta when it exists,
				// create it with a fresh Guid when it does not, then fill entry.id and
				// entry.assetTypeName from the DomainMetaDescriptor.
				// TODO(File): PAL::File::RequestAccess / ReadString / WriteString /
				// ReleaseAccess round trip for that meta file.
				// TODO(Serialization): JsonArchive + Serializer over DomainMetaDescriptor,
				// resolved through Engine's ReflectionSystem.
				// TODO(Cook): produce or refresh the cooked payload for this source and
				// record its path on the snapshot.
			}

			snapshot.entries.PushBack(std::move(entry));
		}

		// TODO(File): delete orphan .hmeta files whose source is gone from this folder.

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