#include "DomainService.h"

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Importer/ImporterContext.h>

#include <Engine/Core/ModuleGraph.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	DomainService::DomainService()
	{
	}

	DomainService::~DomainService()
	{
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

		ProcessPendingImports();
	}

	void DomainService::OnFinalize()
	{
		m_watcher.ClearSubscriptions();
		m_watcher = PAL::DirectoryWatcher();
		m_watcherHealthy = false;

		m_pendingImports.Clear();

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
		if (event.GetExtension() == DomainFile::MetaSuffix)
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
	}

	void DomainService::OnEntryRemoved(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetExtension() == DomainFile::MetaSuffix)
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
		if (event.GetExtension() == DomainFile::MetaSuffix)
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
		if (!EnsureMeta(pFile))
			return;

		if (pFile->GetMeta().assetTypeName.empty())
			return;

		if (pFile->HasBinary())
		{
			RegisterCooked(pFile);
			return;
		}

		QueueImport(pFile->GetID());
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
			Terminal::Error(StringOps::GetName(this), "{} carries no import type attribute", pImporterType->GetName());
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

	void DomainService::QueueImport(const Guid& id)
	{
		if (m_pendingImports.Contains(id))
			return;

		m_pendingImports.PushBack(id);
	}

	void DomainService::ProcessPendingImports()
	{
		if (m_pendingImports.IsEmpty())
			return;

		const Guid id = m_pendingImports.Front();
		m_pendingImports.PopFront();

		DomainFile* pFile = FindFileByGuid(m_root, id);

		if (pFile == nullptr || !pFile->HasSource())
			return;

		if (!IsSourceReady(pFile))
		{
			m_pendingImports.PushBack(id);
			return;
		}

		ImportFile(pFile);
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

	b8 DomainService::ImportFile(DomainFile* pFile)
	{
		AssetImporter* pImporter = m_importerContext->GetImporter(pFile->GetExtension());

		if (pImporter == nullptr)
		{
			Terminal::Warn(StringOps::GetName(this), "{} has no importer", pFile->GetSourcePath());
			return false;
		}

		List<u8> payload;
		const AssetImportResult result = pImporter->ImportAsset(GetEngine(), pFile->GetSourcePath(), payload);

		if (result != AssetImportResult::Success)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be imported, result code is {}", pFile->GetSourcePath(), static_cast<u32>(result));
			return false;
		}

		if (!pFile->WriteCookFile(payload, pImporter->GetPropertySize()))
			return false;

		if (!RegisterCooked(pFile))
			return false;

		++m_revision;

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