#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Project/ProjectContext.h>
#include <Editor/ImportPipeline/ImporterRegistry.h>

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
		Engine::AssetService* pAssetService = GetEngine()->RequestService<Engine::AssetService>();

		if (pAssetService == nullptr)
			return Engine::ModuleReport("Asset service cannot be reached");

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

		m_importPipelineRegistry.BootstrapViews(GetEngine());
		return Engine::ModuleReport();
	}

	void DomainService::OnExecute(const Engine::EngineFrame& ctx)
	{
		if (m_watcher.IsValid())
			m_watcher.Dispatch();
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
		List<std::string> sources;
		m_root->Refresh(sources);

		for (const std::string& relativePath : sources)
		{
			const usize separator = relativePath.rfind('/');
			std::string parentPath = separator == std::string::npos ? std::string() : std::string(relativePath.substr(0, separator));
			std::string fileName = separator == std::string::npos ? relativePath : std::string(relativePath.substr(separator + 1));

			DomainFolder* pParent = m_root->ConfirmAndFindFolder(parentPath);
			if (!pParent)
			{
				Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", parentPath);
				return;
			}

			if (ProcessSourceFile(pParent, relativePath, parentPath, fileName))
				m_revision.Add(1);
		}
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

		// Just add the folder and mock with everyone for recursive folder tracking.
		if (event.kind == PAL::WatcherEntryKind::Directory)
		{
			if (pParent->FindFolder(event.GetName()) != nullptr)
				return;

			List<PAL::Directory::Entry> iterator = PAL::Directory::Iterate(event.absolutePath);
			if (iterator.GetCount())
			{
				Terminal::Warn(StringOps::GetName(this), "{} has multiple items in it and till I add recursive addition, it will skip!", event.absolutePath);
				return;
			}

			DomainFolder* pFolder = pParent->AddFolder(std::string(event.GetName()));
			if (!pFolder)
				return;

			m_revision.Add(1);
			return;
		}

		// Cook the asset file. If it's a success ? add to the domain tree : remove everything created.
		const usize separator = event.relativePath.rfind('/');
		std::string parentPath = separator == std::string::npos ? std::string() : std::string(event.relativePath.substr(0, separator));
		std::string fileName = separator == std::string::npos ? event.relativePath : std::string(event.relativePath.substr(separator + 1));

		// Process the file here
		if (ProcessSourceFile(pParent, event.relativePath, parentPath, fileName))
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

	DomainFile* DomainService::ProcessSourceFile(DomainFolder* pParent, const std::string& relativePath, const std::string& parentPath, const std::string& name)
	{
		if (!pParent)
			return nullptr;

		// If its already there
		if (pParent->FindFile(name))
			return nullptr;

		// Check if it's put on the import already


		// Find the correct import pipeline 
		ImportPipelineEntry* pImportPipelineEntry = m_importPipelineRegistry.GetImportPipelineEntry(name.substr(name.rfind('.')));

		std::string sourcePath = pParent->GetAbsolutePath() + "/" + name;
		std::string metaPath = sourcePath + ".hmeta";

		if (!pImportPipelineEntry)
		{
			Terminal::Warn(StringOps::GetName(this), "{} has no registered import pipeline.", name);
			return nullptr;
		}

		if (!PAL::File::Exists(sourcePath))
			return nullptr;

		// Start processing the condition "If meta is exists" then inner side check for "if source exists"
		if (PAL::File::Exists(metaPath))
		{
			// Get meta
			PAL::FileAccessRequest fileAccess = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

			std::string outStr;
			PAL::File::ReadString(fileAccess, outStr);
			PAL::File::ReleaseAccess(fileAccess);

			JsonArchiveReader jsonReader(outStr);
			jsonReader.BeginObject();

			std::string id;
			if (jsonReader.Key("id"))
				id = jsonReader.ReadString();

			// Cooked asset path
			std::string cookedPath = m_cookPath + "/" + id + ".hasset";
			if (PAL::File::Exists(cookedPath))
			{
				// Register to the asset service
				auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();
				if (!pAssetService->RegisterAsset({ Guid(id), cookedPath, pImportPipelineEntry->pAssetType->GetTypeId() }))
				{
					Terminal::Error(StringOps::GetName(this), "{} has not been able to registered as {} to AssetService", name, id);
					return nullptr;
				}

				//  Create file then create settings
				DomainFile* pFile = pParent->AddFile(Guid(id), name, pImportPipelineEntry->pAssetType);
				if (!pFile->CreateSettings(pImportPipelineEntry->pSettingsType))
				{
					pParent->RemoveEntry(name.substr(name.rfind('.') + 1));
					return nullptr;
				}

				// Deserialize the settings so we can have easy access on editor side
				Serializer settingsSerializer(GetEngine()->GetReflectionSystem(), ResolveType, ResolveTypeByName);

				if (jsonReader.Key("settings"))
				{
					settingsSerializer.Deserialize(pFile->GetSettings(), *pImportPipelineEntry->pSettingsType, jsonReader);
				}
				else
				{
					Terminal::Warn(StringOps::GetName(this), "{} carries no import settings, reimporting with defaults", metaPath);
					QueueImport(pFile, relativePath);
				}

				jsonReader.EndObject();

				if (jsonReader.HasError())
				{
					Terminal::Warn(StringOps::GetName(this), "{} has malformed import settings, reimporting with defaults", metaPath);
					QueueImport(pFile, relativePath);
				}

				return pFile;
			}

			jsonReader.EndObject();

			// Create and send it to queue
			DomainFile* pFile = pParent->AddFile(Guid(id), name, pImportPipelineEntry->pAssetType);
			if (!pFile->CreateSettings(pImportPipelineEntry->pSettingsType))
			{
				pParent->RemoveEntry(name.substr(name.rfind('.') + 1));
				return nullptr;
			}

			QueueImport(pFile, relativePath);
			return nullptr;
		}

		// If not exist, lets have some fun
		Guid newId = Guid::Generate();
		DomainFile* pFile = pParent->AddFile(newId, name, pImportPipelineEntry->pAssetType);
		if (!pFile->CreateSettings(pImportPipelineEntry->pSettingsType))
		{
			pParent->RemoveEntry(name.substr(name.rfind('.') + 1));
			return nullptr;
		}

		QueueImport(pFile, relativePath);
		return nullptr;
	}

	void DomainService::QueueImport(DomainFile* pFile, const std::string& path)
	{
	}

	void DomainService::ImportFile(const std::string& targetFolder, const std::string& importTarget)
	{
	}
}