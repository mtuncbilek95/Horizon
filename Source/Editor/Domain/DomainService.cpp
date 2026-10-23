#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Project/ProjectContext.h>
#include <Editor/ImportPipeline/ImporterRegistry.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>

#include <Engine/Core/ModuleGraph.h>
#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Serialization/Serializer.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	namespace
	{
		const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle)
		{
			return static_cast<Engine::ReflectionSystem*>(pUserData)->GetType(handle);
		}

		const Reflect::Type* ResolveTypeByName(void* pUserData, const std::string& name)
		{
			return static_cast<Engine::ReflectionSystem*>(pUserData)->GetTypeByName(name);
		}
	}

	Engine::ModuleReport DomainService::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();

		auto* pProjectCtx = GetEngine()->RequestContext<ProjectContext>();
		if (pProjectCtx == nullptr)
			return Engine::ModuleReport("Project context cannot be reached");

		m_assetPath = pProjectCtx->GetAssetPath();
		m_cookPath = pProjectCtx->GetCookPath();

		m_importPipelineRegistry.BootstrapViews(GetEngine());

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath, m_cookPath);
		BuildRootTree();

		m_watcher = PAL::DirectoryWatcher(m_assetPath, true);
		BindWatcher();

		return Engine::ModuleReport();
	}

	void DomainService::OnExecute(const Engine::EngineFrame& ctx)
	{
		if (m_watcher.IsValid())
			m_watcher.Dispatch();

		EndImport();
		StartImport();
	}

	void DomainService::OnFinalize()
	{
		m_watcher.ClearSubscriptions();
		m_watcher = PAL::DirectoryWatcher();

		if (m_root == nullptr)
			return;

		Memory::Allocator::Delete(m_root);
	}

	void DomainService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Requires<Engine::AssetService>();
		graph.Requires<ProjectContext>();
	}

	void DomainService::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		m_importPipelineRegistry.OnLibraryRegistered(library);
	}

	void DomainService::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		m_importPipelineRegistry.OnLibraryUnregistered(library);
	}

	void DomainService::BuildRootTree()
	{
		ProcessFolder(m_root);
	}

	void DomainService::BindWatcher()
	{
		m_watcher.OnAdded([this](const PAL::DirectoryWatcher::Event& event)
			{
				OnEntryAdded(event);
			});

		m_watcher.OnModified([this](const PAL::DirectoryWatcher::Event& event)
			{
				OnEntryModified(event);
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

		return m_root->ConfirmAndFindFolder(relativePath);
	}

	DomainFile* DomainService::FindFileByGuid(const Guid& guid) const
	{
		if (m_root == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no domain root", m_assetPath);
			return nullptr;
		}

		DomainFile* pFile = FindFileByGuid(m_root, guid);
		return pFile;
	}


	DomainFile* DomainService::FindFileByRelative(const std::string& relative)
	{
		if (m_root == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no domain root", m_assetPath);
			return nullptr;
		}

		DomainFile* pFile = FindFileByRelative(m_root, relative);
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

	DomainFile* DomainService::FindFileByRelative(DomainFolder* pFolder, const std::string& relative) const
	{
		for (DomainFile* pFile : pFolder->GetFiles())
		{
			if (pFile->GetRelativePath() == relative)
				return pFile;
		}

		for (DomainFolder* pChild : pFolder->GetFolders())
		{
			DomainFile* pFile = FindFileByRelative(pChild, relative);

			if (pFile != nullptr)
				return pFile;
		}

		return nullptr;
	}

	DomainFile* DomainService::FindDomainFile(const std::string& relativePathWithExtension) const
	{
		if (m_root == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no domain root", m_assetPath);
			return nullptr;
		}

		const usize separator = relativePathWithExtension.rfind('/');
		const std::string parentPath = separator == std::string::npos ? std::string() : relativePathWithExtension.substr(0, separator);
		const std::string fileName = separator == std::string::npos ? relativePathWithExtension : relativePathWithExtension.substr(separator + 1);

		DomainFolder* pFolder = m_root->ConfirmAndFindFolder(parentPath);
		if (!pFolder)
			return nullptr;

		return pFolder->FindFileWithExtension(fileName);
	}

	void DomainService::OnEntryAdded(const PAL::DirectoryWatcher::Event& event)
	{
		for (const auto& suffix : IgnoredAddList)
		{
			if (event.GetName().ends_with(suffix))
				return;
		}

		DomainFolder* pParent = m_root->ConfirmAndFindFolder(event.GetParent());
		if (!pParent)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", event.GetParent());
			return;
		}

		if (event.kind == PAL::WatcherEntryKind::Directory)
		{
			if (pParent->FindFolder(event.GetName()) != nullptr)
				return;

			DomainFolder* pFolder = pParent->AddFolder(std::string(event.GetName()));
			if (!pFolder)
				return;

			ProcessFolder(pFolder);
			m_revision.Add(1);
			return;
		}

		// Cook the asset file. If it's a success ? add to the domain tree : remove everything created.
		const usize separator = event.relativePath.rfind('/');
		std::string parentPath = separator == std::string::npos ? std::string() : std::string(event.relativePath.substr(0, separator));
		std::string fileName = separator == std::string::npos ? event.relativePath : std::string(event.relativePath.substr(separator + 1));

		// Process the file here
		if (ProcessSourceFile(pParent, fileName))
			m_revision.Add(1);
	}

	void DomainService::OnEntryModified(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.kind == PAL::WatcherEntryKind::Directory)
			return;

		if (event.relativePath.ends_with(DomainFile::MetaSuffix))
			ReconcileMeta(event.relativePath);
		else
			ReconcilePath(event.relativePath);
	}

	void DomainService::OnEntryRemoved(const PAL::DirectoryWatcher::Event& event)
	{
		// Just go nuts and delete the fuck out everything.
		if (event.relativePath.ends_with(DomainFile::MetaSuffix))
			ReconcileMeta(event.relativePath);
		else
			ReconcilePath(event.relativePath);
	}

	void DomainService::OnEntryRenamed(const PAL::DirectoryWatcher::Event& event)
	{
		for (const auto& suffix : IgnoredAddList)
		{
			if (event.GetName().ends_with(suffix))
				return;
		}

		// Rename meta and source, then change the list for easy access of assets on AssetService.
		DomainFolder* pOldParent = m_root->ConfirmAndFindFolder(event.GetOldParent());
		DomainFolder* pNewParent = m_root->ConfirmAndFindFolder(event.GetParent());

		if (!pOldParent || !pNewParent)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", event.relativePath);
			return;
		}

		const std::string oldName = std::string(event.GetOldName());
		const std::string newName = std::string(event.GetName());
		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();

		if (event.kind == PAL::WatcherEntryKind::Directory)
		{
			// I really dont wanna write a priv function for this motherfucker
			auto unregisterFolder = [pAssetService](this auto& self, DomainFolder* pFolder) -> void
				{
					for (DomainFile* pFile : pFolder->GetFiles())
					{
						if (pAssetService->HasAsset(pFile->GetID()))
							pAssetService->UnregisterAsset(pFile->GetID());
					}

					for (DomainFolder* pChild : pFolder->GetFolders())
						self(pChild);
				};

			if (DomainFolder* pFolder = pOldParent->FindFolder(oldName))
			{
				unregisterFolder(pFolder);
				pOldParent->RemoveEntry(oldName);
			}

			ProcessFolder(pNewParent->AddFolder(newName));
			m_revision.Add(1);
			return;
		}

		DomainFile* pFile = pOldParent->FindFileWithExtension(oldName);
		if (!pFile)
		{
			ReconcilePath(event.relativePath);
			return;
		}

		ImportPipelineEntry* pOldEntry = m_importPipelineRegistry.GetImportPipelineEntry(pFile->GetExtension());
		ImportPipelineEntry* pNewEntry = m_importPipelineRegistry.GetImportPipelineEntry(StringOps::OnlyExtension(newName));

		if (!pNewEntry || pNewEntry != pOldEntry)
		{
			ReconcilePath(event.oldRelativePath);
			ReconcilePath(event.relativePath);
			return;
		}

		const std::string& oldMetaPath = pFile->GetMetaPath();
		const std::string newMetaPath = m_assetPath + "/" + event.relativePath + std::string(DomainFile::MetaSuffix);

		if (!PAL::File::Exists(newMetaPath))
		{
			if (!PAL::File::Exists(oldMetaPath))
				WriteMetaFile(pFile, pNewEntry->pSettingsType);
			else if (!PAL::File::Rename(oldMetaPath, newMetaPath))
				Terminal::Warn(StringOps::GetName(this), "{} could not be renamed to {}", oldMetaPath, newMetaPath);
		}

		if (pAssetService->HasAsset(pFile->GetID()))
			pAssetService->UnregisterAsset(pFile->GetID());

		pOldParent->RemoveFile(pFile);

		if (ProcessSourceFile(pNewParent, newName))
			m_revision.Add(1);
	}

	void DomainService::OnWatcherOverflow()
	{
	}

	void DomainService::RunParseAsset(DomainFileImportContext& task)
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(task.context.sourcePath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!request.IsValid())
		{
			task.result = AssetImportResult::FileNotFound;
			return;
		}

		List<u8> bytes;
		const b8 wasRead = PAL::File::ReadMemory(request, bytes);
		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			task.result = AssetImportResult::FileReadFailed;
			return;
		}

		task.result = task.pImporter->Parse(task.context, ReadOnlyList<u8>(bytes));
	}

	void DomainService::OnParseAssetFinished(usize slot)
	{
		DomainFileImportContext& task = m_runningImports[slot];

		// If its not succeeded something happened.
		if (task.result != AssetImportResult::Success)
		{
			auto* pType = GetEngine()->GetReflectionSystem()->GetType(Reflect::TypeOf<AssetImportResult>());
			Terminal::Error(StringOps::GetName(this), "Parse task for {} has failed with result {}", task.relativePath, pType->GetEnumName(task.result));

			// Since we already created it, re-find the failed domain file.
			DomainFile* pFailed = FindFileByGuid(task.context.fileId);
			const b8 requeue = task.requeue;

			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();

			if (!pFailed)
				return;

			if (requeue)
				QueueImport(pFailed, ImportReason::SourceChanged);
			else if (!GetEngine()->RequestService<Engine::AssetService>()->HasAsset(pFailed->GetID()))
				DiscardFile(pFailed);

			return;
		}

		// If it can't be found, then its deleted.
		auto* pFile = FindFileByGuid(task.context.fileId);
		if (!pFile)
		{
			Terminal::Warn(StringOps::GetName(this), "DomainFile for {} has been deleted during parse", task.relativePath);
			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();

			return;
		}

		// If the pipeline is not there, skip it!
		ImportPipelineEntry* pEntry = m_importPipelineRegistry.GetImportPipelineEntry(pFile->GetExtension());
		if (!pEntry)
		{
			Terminal::Warn(StringOps::GetName(this), "{} has no pipeline for its asset type, skipped", task.relativePath);
			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();

			return;
		}

		task.pPipeline = pEntry->pPipeline;
		task.pAssetType = pFile->GetAssetType();
		task.cookedPath = pFile->GetCookedPath();

		// Start cooking
		task.phase = ImportPhase::Cooking;
		task.ticket = m_jobSystem->SubmitJob(Engine::JobLane::Background, Engine::Job([this, &task]()
			{
				RunCookAsset(task);
			}));

		// Clean the shit up
		if (task.ticket == Engine::InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} cook job could not be submitted", task.relativePath);
			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();
		}
	}

	void DomainService::RunCookAsset(DomainFileImportContext& task)
	{
		ImportProductBytes bytes;

		task.result = task.pImporter->GenerateProperties(task.context, bytes.properties);

		if (task.result == AssetImportResult::Success)
			task.result = task.pImporter->GenerateTable(task.context, bytes.table);

		if (task.result == AssetImportResult::Success)
			task.result = task.pImporter->GenerateBinary(task.context, bytes.binary);

		if (task.result == AssetImportResult::Success)
			task.result = task.pPipeline->Cook(task.context, task.context.fileId, task.pAssetType, task.cookedPath, bytes);
	}

	void DomainService::OnCookAssetFinished(usize slot)
	{
		DomainFileImportContext& task = m_runningImports[slot];

		// If file is not there, blow it up
		DomainFile* pFile = FindFileByGuid(task.context.fileId);
		if (!pFile)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no domain file anymore. Cleaning up the desk!", task.relativePath);

			if (PAL::File::Exists(task.cookedPath) && !PAL::File::Delete(task.cookedPath))
				Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", task.cookedPath);

			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();
			return;
		}

		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
		if (task.result != AssetImportResult::Success)
		{
			auto* pType = GetEngine()->GetReflectionSystem()->GetType(Reflect::TypeOf<AssetImportResult>());
			Terminal::Error(StringOps::GetName(this), "{} has failed on import with code: {}", task.relativePath, pType->GetEnumName(task.result));
		}
		else if (pAssetService->HasAsset(pFile->GetID()))
			pAssetService->RefreshAsset(pFile->GetID());
		else if (!pAssetService->RegisterAsset(pFile->GetID(), task.cookedPath, pFile->GetCallPath(), pFile->GetAssetName(), task.pAssetType->GetTypeId()))
			Terminal::Error(StringOps::GetName(this), "{} could not be registered to AssetService", task.cookedPath);

		const b8 requeue = task.requeue;
		const b8 isDiscarded = task.result != AssetImportResult::Success && !requeue && !pAssetService->HasAsset(pFile->GetID());

		// Write hmeta
		if (!isDiscarded)
			WriteMetaFile(pFile, task.pSettingsType);

		m_revision.Add(1);

		// Get ready to release the task
		Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
		Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
		task = DomainFileImportContext();

		// Discard the file since requeue is also screwed
		if (isDiscarded)
		{
			DiscardFile(pFile);
			return;
		}

		// If there is a requeue, means source file has changed
		if (requeue)
			QueueImport(pFile, ImportReason::SourceChanged);
	}

	void DomainService::ForgetFolder(DomainFolder* pFolder)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
			ForgetFile(pFile);

		for (DomainFolder* pChild : pFolder->GetFolders())
			ForgetFolder(pChild);
	}

	void DomainService::ForgetFile(DomainFile* pFile)
	{
		if (!pFile->GetID().IsValid())
			return;

		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
		pAssetService->UnregisterAsset(pFile->GetID());

		const std::string cookedPath = pFile->GetCookedPath();

		if (PAL::File::Exists(cookedPath) && !PAL::File::Delete(cookedPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", cookedPath);
	}

	void DomainService::DiscardFile(DomainFile* pFile)
	{
		const std::string cookedPath = pFile->GetCookedPath();
		if (PAL::File::Exists(cookedPath) && !PAL::File::Delete(cookedPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", cookedPath);

		const std::string& metaPath = pFile->GetMetaPath();
		if (PAL::File::Exists(metaPath) && !PAL::File::Delete(metaPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", metaPath);

		Terminal::Warn(StringOps::GetName(this), "{} could not be imported and is removed from the domain", pFile->GetSourceRelativePath());

		pFile->GetParent()->RemoveFile(pFile);
		m_revision.Add(1);
	}

	void DomainService::ProcessFolder(DomainFolder* pFolder)
	{
		// Refresh the target folder, it will process the folders by itself.
		List<std::string> sources;
		pFolder->Refresh(sources);

		// Resolve the sources 
		for (const std::string& relativePath : sources)
		{
			const usize separator = relativePath.rfind('/');
			std::string parentPath = separator == std::string::npos ? std::string() : std::string(relativePath.substr(0, separator));
			std::string fileName = separator == std::string::npos ? relativePath : std::string(relativePath.substr(separator + 1));

			DomainFolder* pParent = m_root->ConfirmAndFindFolder(parentPath);
			if (!pParent)
			{
				Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", parentPath);
				continue;
			}

			if (ProcessSourceFile(pParent, fileName))
				m_revision.Add(1);
		}
	}

	DomainFile* DomainService::ProcessSourceFile(DomainFolder* pParent, const std::string& name)
	{
		if (!pParent)
			return nullptr;

		// If its already there
		if (pParent->FindFile(name))
			return nullptr;

		// Find the correct import pipeline 
		ImportPipelineEntry* pImportPipelineEntry = m_importPipelineRegistry.GetImportPipelineEntry(name.substr(name.rfind('.')));

		std::string sourcePath = pParent->GetAbsolutePath() + "/" + name;
		std::string metaPath = sourcePath + ".hmeta";

		if (!pImportPipelineEntry)
		{
			Terminal::Debug(StringOps::GetName(this), "{} has no registered import pipeline.", name);
			return nullptr;
		}

		if (!PAL::File::Exists(sourcePath))
			return nullptr;

		const std::string stem = name.substr(0, name.rfind('.'));

		// If not exist, lets have some fun
		if (!PAL::File::Exists(metaPath))
		{
			DomainFile* pFile = pParent->AddFile(Guid::Generate(), name, pImportPipelineEntry->pAssetType);
			if (!pFile->CreateSettings(pImportPipelineEntry->pSettingsType))
			{
				pParent->RemoveEntry(stem);
				return nullptr;
			}

			QueueImport(pFile, ImportReason::Fresh);
			return pFile;
		}

		// Get meta
		PAL::FileAccessRequest fileAccess = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		std::string outStr;
		PAL::File::ReadString(fileAccess, outStr);
		PAL::File::ReleaseAccess(fileAccess);

		JsonArchiveReader jsonReader(outStr);
		jsonReader.BeginObject();

		Guid id;
		if (jsonReader.Key("id"))
			id = Guid(jsonReader.ReadString());

		if (!id.IsValid())
		{
			Terminal::Warn(StringOps::GetName(this), "{} carries no valid id, a new one is generated", metaPath);
			id = Guid::Generate();
		}

		//  Create file then create settings
		DomainFile* pFile = pParent->AddFile(id, name, pImportPipelineEntry->pAssetType);
		if (!pFile->CreateSettings(pImportPipelineEntry->pSettingsType))
		{
			pParent->RemoveEntry(stem);
			return nullptr;
		}

		// Deserialize the settings so we can have easy access on editor side
		const b8 settingsValid = ReadSettings(pFile, pImportPipelineEntry->pSettingsType, jsonReader);
		jsonReader.EndObject();

		if (!settingsValid)
		{
			QueueImport(pFile, ImportReason::Fresh);
			return pFile;
		}

		// Every product needs its cook on disk, otherwise the whole file goes back to the oven
		if (!PAL::File::Exists(pFile->GetCookedPath()))
		{
			QueueImport(pFile, ImportReason::Fresh);
			return pFile;
		}

		// Register to the asset service
		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
		if (!pAssetService->RegisterAsset(pFile->GetID(), pFile->GetCookedPath(), pFile->GetCallPath(), pFile->GetAssetName(), pFile->GetAssetType()->GetTypeId()))
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be registered to AssetService", pFile->GetName());
			QueueImport(pFile, ImportReason::Fresh);
		}

		return pFile;
	}

	b8 DomainService::ReadSettings(DomainFile* pFile, Reflect::Type* pSettingsType, JsonArchiveReader& reader)
	{
		if (!reader.Key("settings"))
		{
			Terminal::Warn(StringOps::GetName(this), "{} carries no import settings, defaults are used", pFile->GetMetaPath());
			return false;
		}

		Serializer serializer(GetEngine()->GetReflectionSystem(), ResolveType, ResolveTypeByName);
		serializer.Deserialize(pFile->GetSettings(), *pSettingsType, reader);

		if (reader.HasError())
		{
			Terminal::Warn(StringOps::GetName(this), "{} has malformed import settings, defaults are used", pFile->GetMetaPath());
			return false;
		}

		return true;
	}

	b8 DomainService::WriteMetaFile(DomainFile* pFile, Reflect::Type* pSettingsType)
	{
		// Serializer is here to save you
		Serializer serializer(GetEngine()->GetReflectionSystem(), ResolveType, ResolveTypeByName);

		JsonArchiveWriter writer;
		writer.BeginObject();

		writer.Key("id");
		writer.WriteString(pFile->GetID().ToString());

		writer.Key("assetType");
		writer.WriteString(pFile->GetAssetType()->GetName());

		writer.Key("settings");
		serializer.Serialize(pFile->GetSettings(), *pSettingsType, writer);

		writer.EndObject();

		const std::string& metaPath = pFile->GetMetaPath();
		if (!PAL::File::Exists(metaPath) && !PAL::File::Create(metaPath))
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be created", metaPath);
			return false;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);
		if (!request.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be opened for writing", metaPath);
			return false;
		}

		const b8 wasWritten = PAL::File::Truncate(request, 0) && PAL::File::WriteString(request, writer.ToString());
		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
			Terminal::Error(StringOps::GetName(this), "{} could not be written", metaPath);

		return wasWritten;
	}

	void DomainService::ReconcilePath(const std::string& relativePath)
	{
		const usize separator = relativePath.rfind('/');
		const std::string parentPath = separator == std::string::npos ? std::string() : relativePath.substr(0, separator);
		const std::string name = separator == std::string::npos ? relativePath : relativePath.substr(separator + 1);

		DomainFolder* pParent = m_root->ConfirmAndFindFolder(parentPath);
		if (!pParent)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", parentPath);
			return;
		}

		const std::string absolutePath = m_assetPath + "/" + relativePath;

		if (DomainFolder* pFolder = pParent->FindFolder(name))
		{
			if (PAL::Directory::Exists(absolutePath))
				return;

			ForgetFolder(pFolder);
			pParent->RemoveEntry(name);
			m_revision.Add(1);
			return;
		}

		DomainFile* pFile = pParent->FindFileWithExtension(name);
		const b8 isOnDisk = PAL::File::Exists(absolutePath);

		if (!pFile)
		{
			if (isOnDisk && ProcessSourceFile(pParent, name))
				m_revision.Add(1);

			return;
		}

		if (isOnDisk)
		{
			QueueImport(pFile, ImportReason::SourceChanged);
			return;
		}

		ForgetFile(pFile);

		const std::string& metaPath = pFile->GetMetaPath();
		if (PAL::File::Exists(metaPath) && !PAL::File::Delete(metaPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", metaPath);

		pParent->RemoveFile(pFile);
		m_revision.Add(1);
	}

	void DomainService::ReconcileMeta(const std::string& relativePath)
	{
		DomainFile* pFile = FindDomainFile(relativePath.substr(0, relativePath.size() - DomainFile::MetaSuffix.size()));
		if (!pFile)
			return;

		ImportPipelineEntry* pEntry = m_importPipelineRegistry.GetImportPipelineEntry(pFile->GetExtension());
		if (!pEntry)
		{
			Terminal::Debug(StringOps::GetName(this), "{} has no registered import pipeline.", pFile->GetName());
			return;
		}

		if (!PAL::File::Exists(pFile->GetMetaPath()))
		{
			if (!PAL::File::Exists(pFile->GetSourcePath()))
				return;

			Terminal::Warn(StringOps::GetName(this), "{} is deleted while its source is tracked, it is written back", pFile->GetMetaPath());
			WriteMetaFile(pFile, pEntry->pSettingsType);
			return;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(pFile->GetMetaPath(), PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!request.IsValid())
		{
			Terminal::Warn(StringOps::GetName(this), "{} could not be opened for reading", pFile->GetMetaPath());
			return;
		}

		std::string text;
		PAL::File::ReadString(request, text);
		PAL::File::ReleaseAccess(request);

		Serializer serializer(GetEngine()->GetReflectionSystem(), ResolveType, ResolveTypeByName);

		JsonArchiveWriter before;
		serializer.Serialize(pFile->GetSettings(), *pEntry->pSettingsType, before);

		JsonArchiveReader reader(text);
		reader.BeginObject();
		const b8 isValid = ReadSettings(pFile, pEntry->pSettingsType, reader);
		reader.EndObject();

		if (!isValid)
		{
			JsonArchiveReader restore(before.ToString());
			serializer.Deserialize(pFile->GetSettings(), *pEntry->pSettingsType, restore);
			return;
		}

		JsonArchiveWriter after;
		serializer.Serialize(pFile->GetSettings(), *pEntry->pSettingsType, after);

		if (before.ToString() == after.ToString())
			return;

		QueueImport(pFile, ImportReason::SettingsChanged);
	}

	void DomainService::QueueImport(DomainFile* pFile, ImportReason reason)
	{
		// Check if file is a valid one
		if (!pFile || !pFile->GetSettings())
		{
			Terminal::Error(StringOps::GetName(this), "Import requested for a file without settings");
			return;
		}

		// Check if the requested one is in queue already.
		const Guid& id = pFile->GetID();
		if (DomainFileImportContext* pRunning = FindRunningImport(id))
		{
			pRunning->requeue = true;
			return;
		}

		// Check if its in pending
		if (IsImportPending(id))
			return;

		// Gather everything, we're going to the war
		ImportPipelineEntry* pEntry = m_importPipelineRegistry.GetImportPipelineEntry(pFile->GetExtension());
		if (!pEntry)
			return;

		AssetImporter* pImporter = m_importPipelineRegistry.CreateImporterInstance(pFile->GetExtension());
		if (!pImporter)
		{
			Terminal::Error(StringOps::GetName(this), "{} has a pipeline but no importer for {}", pFile->GetName(), pEntry->pAssetType->GetName());
			return;
		}

		// Take a snapshot, to save your ass for main thread fuckery.
		AssetImportSettings* pSnapshot = CloneSettings(pFile->GetSettings(), pEntry->pSettingsType);
		if (!pSnapshot)
		{
			Terminal::Error(StringOps::GetName(this), "{} import settings could not be snapshotted", pFile->GetName());
			return;
		}

		// Create a pending
		DomainFileImportContext task;
		task.relativePath = pFile->GetSourceRelativePath();
		task.pImporter = pImporter;
		task.pSettings = pSnapshot;
		task.pSettingsType = pEntry->pSettingsType;

		task.context.fileId = id;
		task.context.sourcePath = pFile->GetSourcePath();
		task.context.sourceFolder = pFile->GetParent()->GetAbsolutePath();
		task.context.reason = reason;
		task.context.pSettings = pSnapshot;
		task.context.pReflection = GetEngine()->GetReflectionSystem();

		m_pendingImports.PushBack(std::move(task));
	}

	Reflect::Type* DomainService::GetImportSettingsType(const std::string& extension)
	{
		ImportPipelineEntry* pEntry = m_importPipelineRegistry.GetImportPipelineEntry(extension);
		if (!pEntry)
		{
			Terminal::Warn(StringOps::GetName(this), "{} has no registered import pipeline", extension);
			return nullptr;
		}

		return pEntry->pSettingsType;
	}

	void DomainService::ImportFile(const std::string& targetFolder, const std::string& sourcePath, const AssetImportSettings* pSettings)
	{
		/*const std::string source = StringOps::NormalizePath(sourcePath);
		const usize separator = source.rfind('/');
		const std::string name = separator == std::string::npos ? source : source.substr(separator + 1);

		ImportPipelineEntry* pEntry = m_importPipelineRegistry.GetImportPipelineEntry(StringOps::OnlyExtension(name));
		if (!pEntry)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no registered import pipeline", name);
			return;
		}

		const std::string targetPath = targetFolder + "/" + name;
		const std::string metaPath = targetPath + std::string(DomainFile::MetaSuffix);

		if (PAL::File::Exists(targetPath) || PAL::File::Exists(metaPath))
		{
			Terminal::Error(StringOps::GetName(this), "{} already exists in {}", name, targetFolder);
			return;
		}

		if (!WriteMetaFile(metaPath, Guid::Generate(), pEntry->pAssetType, pSettings, pEntry->pSettingsType))
			return;

		if (PAL::File::Copy(source, targetFolder))
			return;

		if (!PAL::File::Delete(metaPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", metaPath);*/
	}

	void DomainService::StartImport()
	{
		// Here we go
		for (usize slot = 0; slot < MaxConcurrentImports; slot++)
		{
			if (m_pendingImports.GetCount() == 0)
				return;

			DomainFileImportContext& task = m_runningImports[slot];

			if (task.ticket != Engine::InvalidSubmitTicket)
				continue;

			task = std::move(m_pendingImports.Front());
			m_pendingImports.PopFront();

			task.phase = ImportPhase::Parsing;
			task.ticket = m_jobSystem->SubmitJob(Engine::JobLane::Background, Engine::Job([this, &task]()
				{
					RunParseAsset(task);
				}));

			if (task.ticket == Engine::InvalidSubmitTicket)
			{
				Terminal::Error(StringOps::GetName(this), "{} import job could not be submitted", task.relativePath);
				Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
				Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
				task = DomainFileImportContext();
			}
		}
	}

	void DomainService::EndImport()
	{
		for (usize slot = 0; slot < MaxConcurrentImports; slot++)
		{
			DomainFileImportContext& task = m_runningImports[slot];

			// If its still invalid, move on
			if (task.ticket == Engine::InvalidSubmitTicket)
				continue;

			// Not finished, move on
			if (m_jobSystem->GetTicketState(task.ticket) != Engine::CompletionState::Completed)
				continue;

			// Tag as invalid for weird corruptions
			task.ticket = Engine::InvalidSubmitTicket;

			// If in loading state
			if (task.phase == ImportPhase::Parsing)
			{
				OnParseAssetFinished(slot);
				continue;
			}

			// If its cooking
			if (task.phase == ImportPhase::Cooking)
				OnCookAssetFinished(slot);
		}
	}

	DomainFileImportContext* DomainService::FindRunningImport(const Guid& id)
	{
		for (DomainFileImportContext& task : m_runningImports)
		{
			if (task.ticket != Engine::InvalidSubmitTicket && task.context.fileId == id)
				return &task;
		}

		return nullptr;
	}

	b8 DomainService::IsImportPending(const Guid& id) const
	{
		for (const DomainFileImportContext& task : m_pendingImports)
		{
			if (task.context.fileId == id)
				return true;
		}

		return false;
	}

	AssetImportSettings* DomainService::CloneSettings(const AssetImportSettings* pSource, Reflect::Type* pType)
	{
		// Dont forget to create and check if its created properly
		auto* pClone = (AssetImportSettings*)pType->Create();
		if (!pClone)
		{
			Terminal::Error(StringOps::GetName(this), "{} has virtual function issues. Check out your settings", pType->GetName());
			return nullptr;
		}

		Serializer serializer(GetEngine()->GetReflectionSystem(), ResolveType, ResolveTypeByName);

		// Some read and write, huh?
		JsonArchiveWriter writer;
		serializer.Serialize(pSource, *pType, writer);

		JsonArchiveReader reader(writer.ToString());
		serializer.Deserialize(pClone, *pType, reader);

		if (reader.HasError())
		{
			Memory::Allocator::Delete(pClone);
			return nullptr;
		}

		return pClone;
	}
}