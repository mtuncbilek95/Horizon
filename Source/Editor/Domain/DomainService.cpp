#include "DomainService.h"

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Importer/ImporterContext.h>

#include <Engine/Core/ModuleGraph.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	void DomainService::RunImport(ImportTask* pTask)
	{
		Terminal::Info("DomainService", "{} is being imported on thread {}", pTask->sourcePath, PAL::Thread::CurrentId());

		List<u8> content;
		pTask->result = pTask->pImporter->ImportAsset(pTask->pEngine, pTask->sourcePath, content);

		if (pTask->result != AssetImportResult::Success)
			return;

		const std::string tempPath = pTask->cookedPath + ".tmp";

		if (!DomainFile::WriteCookFile(tempPath, pTask->id, pTask->assetTypeName, content, pTask->pImporter->GetPropertySize()))
			return;

		if (PAL::File::Exists(pTask->cookedPath) && !PAL::File::Delete(pTask->cookedPath))
		{
			Terminal::Error("DomainService", "{} cannot be replaced", pTask->cookedPath);
			return;
		}

		pTask->wasCooked = PAL::File::Rename(tempPath, pTask->cookedPath);

		if (pTask->wasCooked)
			Terminal::Info("DomainService", "{} has been imported via thread {} with {} bytes", pTask->sourcePath, PAL::Thread::CurrentId(), content.GetCount());
	}

	Engine::ModuleReport DomainService::OnInitialize()
	{
		Engine::AssetService* pAssetService = GetEngine()->RequestService<Engine::AssetService>();

		if (pAssetService == nullptr)
			return Engine::ModuleReport("Asset service cannot be reached");

		m_importerContext = GetEngine()->RequestContext<ImporterContext>();

		if (m_importerContext == nullptr)
			return Engine::ModuleReport("Importer context cannot be reached");

		// If you see this and judge me, FUCK YOU! IT WILL BE AUTOMATIC PLEASE FUCK OFF!
		m_projectPath = "D:/Projects/Horizon/ExampleProject";
		m_assetPath = m_projectPath + "/Assets";
		m_cookPath = m_projectPath + "/Cooked";

		if (!PAL::Directory::Exists(m_assetPath) && !PAL::Directory::Create(m_assetPath))
			return Engine::ModuleReport("Asset root cannot be created");

		if (!PAL::Directory::Exists(m_cookPath) && !PAL::Directory::Create(m_cookPath))
			return Engine::ModuleReport("Cook root cannot be created");

		m_projectSource = Memory::Allocator::Create<Engine::LooseSourceFile>(Memory::CurrLoc(), "Project");
		pAssetService->AddSource(m_projectSource);

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath, m_cookPath);
		m_root->Refresh();
		TrackFolder(m_root);

		m_watcher = PAL::DirectoryWatcher(m_assetPath, true);
		m_watcherHealthy = m_watcher.IsValid();

		if (!m_watcherHealthy)
		{
			Terminal::Warn(StringOps::GetName(this), "{} cannot be watched, the domain tree will not refresh", m_assetPath);
			return Engine::ModuleReport();
		}

		BindWatcher();

		return Engine::ModuleReport();
	}

	void DomainService::OnExecute(const Engine::EngineFrame& ctx)
	{
		if (m_watcherHealthy)
			m_watcher.Dispatch();

		CommitFinishedImports();
		ProcessPendingImports();
	}

	void DomainService::OnFinalize()
	{
		m_watcher.ClearSubscriptions();
		m_watcher = PAL::DirectoryWatcher();
		m_watcherHealthy = false;

		m_pendingImports.Clear();

		Engine::JobSystem* pJobSystem = GetEngine()->GetJobSystem();

		for (ImportTask* pTask : m_activeImports)
		{
			pJobSystem->WaitTicket(pTask->ticket);
			Memory::Allocator::Delete(pTask);
		}

		m_activeImports.Clear();

		if (m_root == nullptr)
			return;

		Memory::Allocator::Delete(m_root);
		Memory::Allocator::Delete(m_projectSource);
	}

	void DomainService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Requires<Engine::AssetService>();
		graph.Requires<ImporterContext>();
	}

	void DomainService::BindWatcher()
	{
		m_watcher.OnAdded([this](const PAL::DirectoryWatcher::Event& event)
			{
				OnEntryAdded(event);
			});

		m_watcher.OnRemoved([this](const PAL::DirectoryWatcher::Event& event)
			{
				OnEntryRemoved(event);
			});

		m_watcher.OnRenamed([this](const PAL::DirectoryWatcher::Event& event)
			{
				OnEntryRenamed(event);
			});

		m_watcher.OnOverflow([this](const PAL::DirectoryWatcher::Event&)
			{
				OnWatcherOverflow();
			});
	}

	DomainFolder* DomainService::FindFolder(std::string_view relativePath) const
	{
		if (m_root == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no domain root", m_assetPath);
			return nullptr;
		}

		return m_root->ResolveFolder(relativePath);
	}

	DomainFile* DomainService::FindFileByGuid(const Guid& guid) const
	{
		if (m_root == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no domain root", m_assetPath);
			return nullptr;
		}

		DomainFile* pFile = FindFileByGuid(m_root, guid);

		if (pFile == nullptr)
			Terminal::Error(StringOps::GetName(this), "{} is not tracked by any domain file", guid.ToString());

		return pFile;
	}

	DomainFile* DomainService::FindFileByGuid(DomainFolder* pFolder, const Guid& guid) const
	{
		for (DomainFile* pFile : pFolder->GetFiles())
		{
			if (pFile->GetID() == guid)
				return pFile;
		}

		for (DomainFolder* pChild : pFolder->GetFolders())
		{
			DomainFile* pFile = FindFileByGuid(pChild, guid);

			if (pFile != nullptr)
				return pFile;
		}

		return nullptr;
	}

	void DomainService::OnEntryAdded(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;

		DomainFolder* pParent = m_root->ResolveFolder(event.GetParent());

		if (pParent == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", event.GetParent());
			return;
		}

		if (event.kind == PAL::WatcherEntryKind::Directory)
		{
			DomainFolder* pFolder = pParent->AddFolder(std::string(event.GetName()));
			pFolder->Refresh();
			TrackFolder(pFolder);
			++m_revision;
			return;
		}

		DomainFile* pFile = pParent->AddFile(std::string(event.GetName()));

		if (pFile == nullptr)
		{
			Terminal::Warn(StringOps::GetName(this), "{} could not be tracked", event.relativePath);
			return;
		}

		TrackFile(pFile);
		++m_revision;
	}

	void DomainService::OnEntryModified(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;
	}

	void DomainService::OnEntryRemoved(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;

		DomainFolder* pParent = m_root->ResolveFolder(event.GetParent());

		if (pParent == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", event.GetParent());
			return;
		}

		DomainFile* pFile = pParent->FindFile(event.GetName());

		if (pFile != nullptr)
			ForgetFile(pFile);

		if (!pParent->RemoveEntry(event.GetName()))
			return;

		++m_revision;
	}

	void DomainService::OnEntryRenamed(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;

		DomainFolder* pOldParent = m_root->ResolveFolder(event.GetOldParent());

		if (pOldParent != nullptr)
		{
			DomainFile* pOldFile = pOldParent->FindFile(event.GetOldName());

			if (pOldFile != nullptr)
				MoveMeta(pOldFile->GetMetaPath(), m_assetPath + "/" + event.relativePath + std::string(DomainFile::MetaSuffix));

			pOldParent->RemoveEntry(event.GetOldName());
		}

		OnEntryAdded(event);
	}

	void DomainService::OnWatcherOverflow()
	{
		Terminal::Warn(StringOps::GetName(this), "{} overflowed its notifications, the domain tree is rebuilt", m_assetPath);

		m_root->Refresh();
		TrackFolder(m_root);
		++m_revision;
	}

	void DomainService::TrackFolder(DomainFolder* pFolder)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
			TrackFile(pFile);

		for (DomainFolder* pChild : pFolder->GetFolders())
			TrackFolder(pChild);
	}

	void DomainService::TrackFile(DomainFile* pFile)
	{
		// Check if we have meta and load the fuck out of it, if not generate
		if (!EnsureMeta(pFile))
			return;

		if (pFile->GetMeta().assetTypeName.empty())
			return;

		if (pFile->HasBinary())
		{
			// TODO: Having binary doesn't mean its valid
			RegisterCooked(pFile);
			return;
		}

		if (m_pendingImports.Contains(pFile->GetID()))
			return;

		// Just add to import job tracking list.
		m_pendingImports.PushBack(pFile->GetID());
	}

	void DomainService::ForgetFile(DomainFile* pFile)
	{
		const Guid id = pFile->GetID();

		if (!id.IsValid())
			return;

		m_pendingImports.Remove(id);

		if (m_projectSource->Find(id) != nullptr)
			m_projectSource->Unregister(id);
	}

	b8 DomainService::EnsureMeta(DomainFile* pFile)
	{
		if (pFile->HasMeta())
		{
			if (!pFile->LoadMetaFile())
			{
				Terminal::Warn(StringOps::GetName(this), "{} has a meta that cannot be loaded, it is left untouched", pFile->GetSourcePath());
				return false;
			}

			if (!pFile->GetMeta().assetTypeName.empty())
				return true;

			DomainMeta meta = pFile->GetMeta();
			meta.assetTypeName = ResolveAssetTypeName(pFile);

			if (meta.assetTypeName.empty())
				return true;

			return pFile->WriteMetaFile(meta);
		}

		DomainMeta meta;
		meta.id = Guid::Generate();
		meta.assetTypeName = ResolveAssetTypeName(pFile);

		if (!pFile->WriteMetaFile(meta))
		{
			Terminal::Warn(StringOps::GetName(this), "{} could not get a meta", pFile->GetSourcePath());
			return false;
		}

		return true;
	}

	std::string DomainService::ResolveAssetTypeName(DomainFile* pFile)
	{
		// TODO: I guess if std::string is empty the DomainFile that we're working on should be deleted instead 
		// of being imported weirdly.
		
		// This function just takes MeshAsset, Texture2DAsset, AnimationAsset, RenderGraphAsset etc.
		// And it takes from extension + importer. I couldn't find a better way.
		AssetImporter* pImporter = m_importerContext->GetImporter(pFile->GetExtension());

		if (pImporter == nullptr)
			return std::string();

		auto* pReflect = GetEngine()->GetReflectionSystem();

		Reflect::Type* pImporterType = pReflect->GetType(pImporter->GetTypeId());

		if (pImporterType == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} importer has no reflected type", pFile->GetExtension());
			return std::string();
		}

		ImportTypeAttribute* pAttr = pImporterType->GetCustomAttribute<ImportTypeAttribute>();

		if (pAttr == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no ImportTypeAttribute", pImporterType->GetName());
			return std::string();
		}

		Reflect::Type* pAssetType = pReflect->GetType(pAttr->GetType());

		if (pAssetType == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} imports an asset type that is not reflected", pImporterType->GetName());
			return std::string();
		}

		return pAssetType->GetName();
	}

	void DomainService::MoveMeta(const std::string& oldMetaPath, const std::string& newMetaPath)
	{
		if (!PAL::File::Exists(oldMetaPath))
			return;

		if (PAL::File::Exists(newMetaPath))
			return;

		if (!PAL::File::Rename(oldMetaPath, newMetaPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not follow its source to {}", oldMetaPath, newMetaPath);
	}

	void DomainService::ProcessPendingImports()
	{
		if (m_pendingImports.IsEmpty() || m_activeImports.GetCount() >= MaxConcurrentImports)
			return;

		const Guid id = m_pendingImports.Front();
		m_pendingImports.PopFront();

		if (IsImportActive(id))
		{
			m_pendingImports.PushBack(id);
			return;
		}

		DomainFile* pFile = FindFileByGuid(m_root, id);

		if (pFile == nullptr || !pFile->HasSource())
			return;

		if (!IsSourceReady(pFile))
		{
			m_pendingImports.PushBack(id);
			return;
		}

		StartImport(pFile);
	}

	void DomainService::CommitFinishedImports()
	{
		Engine::JobSystem* pJobSystem = GetEngine()->GetJobSystem();

		for (usize i = 0; i < m_activeImports.GetCount();)
		{
			ImportTask* pTask = m_activeImports[i];
			const Engine::CompletionState state = pJobSystem->GetTicketState(pTask->ticket);

			if (state == Engine::CompletionState::Pending || state == Engine::CompletionState::Running)
			{
				++i;
				continue;
			}

			m_activeImports.RemoveAt(i);

			DomainFile* pFile = FindFileByGuid(m_root, pTask->id);

			if (pTask->result != AssetImportResult::Success)
				Terminal::Error(StringOps::GetName(this), "{} could not be imported, result code is {}", pTask->sourcePath, static_cast<u32>(pTask->result));
			else if (!pTask->wasCooked)
				Terminal::Error(StringOps::GetName(this), "{} was imported but could not be cooked", pTask->sourcePath);
			else if (pFile == nullptr)
			{
				Terminal::Warn(StringOps::GetName(this), "{} vanished while it was being imported", pTask->sourcePath);
				PAL::File::Delete(pTask->cookedPath);
			}
			else if (RegisterCooked(pFile))
				++m_revision;

			Memory::Allocator::Delete(pTask);
		}
	}

	b8 DomainService::IsImportActive(const Guid& id) const
	{
		for (const ImportTask* pTask : m_activeImports)
		{
			if (pTask->id == id)
				return true;
		}

		return false;
	}

	b8 DomainService::IsSourceReady(DomainFile* pFile) const
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(pFile->GetSourcePath(), PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::Exclusive);

		if (!request.IsValid())
			return false;

		PAL::File::ReleaseAccess(request);

		return true;
	}

	b8 DomainService::StartImport(DomainFile* pFile)
	{
		AssetImporter* pImporter = m_importerContext->GetImporter(pFile->GetExtension());

		if (pImporter == nullptr)
		{
			Terminal::Warn(StringOps::GetName(this), "{} has no importer", pFile->GetSourcePath());
			return false;
		}

		const std::string cookedPath = pFile->GetCookedPath();

		if (cookedPath.empty())
			return false;

		ImportTask* pTask = Memory::Allocator::Create<ImportTask>(Memory::CurrLoc());
		pTask->id = pFile->GetID();
		pTask->sourcePath = pFile->GetSourcePath();
		pTask->cookedPath = cookedPath;
		pTask->assetTypeName = pFile->GetMeta().assetTypeName;
		pTask->pImporter = pImporter;
		pTask->pEngine = GetEngine();

		pTask->ticket = GetEngine()->GetJobSystem()->SubmitJob(Engine::JobLane::Background, Engine::Job([pTask]()
			{
				RunImport(pTask);
			}));

		if (pTask->ticket == Engine::InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be submitted as an import job", pTask->sourcePath);
			Memory::Allocator::Delete(pTask);
			return false;
		}

		m_activeImports.PushBack(pTask);

		return true;
	}

	b8 DomainService::RegisterCooked(DomainFile* pFile)
	{
		Reflect::Type* pType = GetEngine()->GetReflectionSystem()->GetTypeByName(pFile->GetMeta().assetTypeName);

		if (pType == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} names an asset type that is not reflected", pFile->GetMetaPath());
			return false;
		}

		Engine::AssetEntry entry = {};
		entry.assetId = pFile->GetID();
		entry.cookedPath = pFile->GetCookedPath();
		entry.assetTypeHandle = pType->GetTypeId();

		return m_projectSource->Register(entry);
	}
}