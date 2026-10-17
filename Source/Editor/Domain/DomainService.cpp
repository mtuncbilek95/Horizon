#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
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

	DomainFile* DomainService::FindSourceFile(const std::string& relativePathWithExtension)
	{
		if (!PAL::File::Exists(m_assetPath + "/" + relativePathWithExtension))
			return nullptr;

		const usize separator = relativePathWithExtension.rfind('/');
		const std::string parentPath = separator == std::string::npos ? std::string() : relativePathWithExtension.substr(0, separator);
		const std::string fileName = separator == std::string::npos ? relativePathWithExtension : relativePathWithExtension.substr(separator + 1);

		DomainFolder* pFolder = m_root;
		usize offset = 0;

		while (offset < parentPath.size())
		{
			const usize next = parentPath.find('/', offset);
			const usize count = next == std::string::npos ? parentPath.size() - offset : next - offset;

			pFolder = pFolder->AddFolder(parentPath.substr(offset, count));
			if (!pFolder)
				return nullptr;

			offset = next == std::string::npos ? parentPath.size() : next + 1;
		}

		return ProcessSourceFile(pFolder, fileName);
	}

	void DomainService::OnEntryAdded(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;

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

		// Check if the source or meta file has modified.

		// If source file has modified: Re-cook

		// If meta file has modified: Trigger Re-configure which modifies the AssetProperties.
	}

	void DomainService::OnEntryRemoved(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;

		// Just go nuts and delete the fuck out everything.
	}

	void DomainService::OnEntryRenamed(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetName().ends_with(DomainFile::MetaSuffix))
			return;

		// Rename meta and source, then change the list for easy access of assets on AssetService.
	}

	void DomainService::OnWatcherOverflow()
	{
	}

	void DomainService::RunParseAsset(DomainFileImportContext& task)
	{
	}

	void DomainService::OnParseAssetFinished(usize slot)
	{
		DomainFileImportContext& task = m_runningImports[slot];
		
		// If its not succeeded something happened.
		if (task.result != AssetImportResult::Success)
		{
			Terminal::Error(StringOps::GetName(this), "Parse task for {} has failed with result {}", task.relativePath, u8(task.result));
			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();

			return;
		}

		// If it can't be found, then its deleted.
		auto* pFile = FindFileByGuid(task.id);
		if (!pFile)
		{
			Terminal::Warn(StringOps::GetName(this), "DomainFile for {} has been deleted during parse", task.relativePath);
			Memory::Allocator::Delete(task.pImporter); // Don't forget to delete this mf
			Memory::Allocator::Delete(task.pSettings); // Don't forget to delete this mf
			task = DomainFileImportContext();

			return;
		}

		std::string folderRelative = pFile->GetParent()->GetRelativePath();
		List<DomainFile::DomainDependency> entries;

		for (ImportDependency& dependency : task.dependencies)
		{
			// Resolve the path via DomainFile's relativePath like "../textures/myTexture.png" to "TestObjects/MyObject/textures/myTexture.png",
			// so we can refer them and their guid properly get in the meta file and other places.
			dependency.targetRelativePath = StringOps::ResolveRelativePath(folderRelative, dependency.sourceRelativePath);

			DomainFile::DomainDependency entry;
			entry.relativePath = dependency.targetRelativePath;
			entry.isAsset = dependency.kind == ImportDependencyKind::Asset;

			if (dependency.kind == ImportDependencyKind::Data)
			{
				entry.resolved = true;
				entries.PushBack(std::move(entry));
				continue;
			}

			DomainFile* pDependency = FindFileByRelative(dependency.targetRelativePath);

			// If dependent domain file is not there, try to find in the path in case its not become a DomainFile yet.
			if (!pDependency)
				pDependency = FindSourceFile(dependency.targetRelativePath);

			if (pDependency)
				dependency.resolvedId = pDependency->GetSubAssetId(dependency.subAssetKey);

			entry.resolved = dependency.resolvedId.IsValid();
			if (!entry.resolved)
				Terminal::Warn(StringOps::GetName(this), "{} depends on {} which is not available yet, cooked with an empty reference", task.relativePath, dependency.targetRelativePath);

			entries.PushBack(std::move(entry));
		}

		// Dependencies are ready
		pFile->SetDependencies(entries);

		List<Guid> removed;
		// Renew the sub assets
		pFile->SyncSubAssets(task.expectedProducts, GetEngine()->GetReflectionSystem(), removed);

		// Unregister the removed ones
		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
		for (const Guid& id : removed)
		{
			pAssetService->UnregisterAsset(id);

			const std::string cookedPath = pFile->GetSubAssetCookedPath(id);
			if (PAL::File::Exists(cookedPath) && !PAL::File::Delete(cookedPath))
				Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", cookedPath);
		}

		task.cookedProducts.Clear();
		task.cookedProducts.Reserve(task.expectedProducts.GetCount());

		// Get the expected products to cooked ones 
		for (const ImportDeclaredProduct& declared : task.expectedProducts)
		{
			DomainCookedProduct product;
			product.key = declared.key;
			product.id = pFile->GetSubAssetId(declared.key);
			product.pAssetType = GetEngine()->GetReflectionSystem()->GetType(declared.assetType);
			product.cookedPath = pFile->GetSubAssetCookedPath(product.id);

			ImportPipelineEntry* pEntry = m_importPipelineRegistry.GetImportPipelineEntry(declared.assetType);
			product.pPipeline = pEntry ? pEntry->pPipeline : nullptr;

			task.cookedProducts.PushBack(std::move(product));
		}

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
	}

	void DomainService::OnCookAssetFinished(usize slot)
	{
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
		const Guid id = pFile->GetID();

		if (!id.IsValid())
			return;

		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
		pAssetService->UnregisterAsset(id);

		const std::string cookedPath = m_cookPath + "/" + id.ToString();

		if (PAL::File::Exists(cookedPath) && !PAL::File::Delete(cookedPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted", cookedPath);
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

		// Cooked asset path
		const std::string cookedPath = pFile->GetCookedPath();
		if (!PAL::File::Exists(cookedPath) || !settingsValid)
		{
			//return pFile;
			/*QueueImport(pFile, ImportReason::Fresh);
			return pFile;*/
		}

		// Register to the asset service
		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
		if (!pAssetService->RegisterAsset({ id, cookedPath, pImportPipelineEntry->pAssetType->GetTypeId() }))
		{
			Terminal::Error(StringOps::GetName(this), "{} has not been able to registered as {} to AssetService", name, id.ToString());
			pParent->RemoveEntry(stem);
			return nullptr;
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
		/*if (DomainFileImportContext* pRunning = FindRunningImport(id))
		{
			pRunning->requeue = true;
			return;
		}*/

		// Check if its in pending
		/*if (IsImportPending(id))
			return;*/

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
		task.id = id;
		task.relativePath = pFile->GetRelativePath() + pFile->GetExtension();
		task.sourcePath = pFile->GetSourcePath();
		task.cookedPath = pFile->GetCookedPath();
		task.pPipeline = pEntry->pPipeline;
		task.pImporter = pImporter;
		task.pAssetType = pEntry->pAssetType;
		task.pSettingsType = pEntry->pSettingsType;
		task.pSettings = pSnapshot;
		task.reason = reason;

		m_pendingImports.PushBack(std::move(task));
	}

	void DomainService::ImportFile(const std::string& targetFolder, const std::string& importTarget)
	{
	}

	void DomainService::StartImport()
	{
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
					// Run the fuckin parser
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
				OnParseAssetFinished(slot);
		}
	}

	DomainFileImportContext* DomainService::FindRunningImport(const Guid& id)
	{
		for (DomainFileImportContext& task : m_runningImports)
		{
			if (task.ticket != Engine::InvalidSubmitTicket && task.id == id)
				return &task;
		}

		return nullptr;
	}

	b8 DomainService::IsImportPending(const Guid& id) const
	{
		for (const DomainFileImportContext& task : m_pendingImports)
		{
			if (task.id == id)
				return true;
		}

		return false;
	}

	AssetImportSettings* DomainService::CloneSettings(const AssetImportSettings* pSource, Reflect::Type* pType)
	{
		auto* pClone = (AssetImportSettings*)pType->Create();
		if (!pClone)
			return nullptr;

		Serializer serializer(GetEngine()->GetReflectionSystem(), ResolveType, ResolveTypeByName);

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