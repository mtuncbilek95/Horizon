#include "DomainService.h"

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/DomainMeta.h>
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

	void DomainService::SplitPath(std::string_view relativePath, std::string& outParent, std::string& outName)
	{
		const usize separator = relativePath.find_last_of('/');

		if (separator == std::string_view::npos)
		{
			outParent.clear();
			outName = std::string(relativePath);
			return;
		}

		outParent = std::string(relativePath.substr(0, separator));
		outName = std::string(relativePath.substr(separator + 1));
	}

	std::string DomainService::ExtensionOf(const std::string& name)
	{
		std::string extension = StringOps::OnlyExtension(name);

		for (c8& character : extension)
			character = StringOps::ToLowerAscii(character);

		return extension;
	}

	Engine::ModuleReport DomainService::OnInitialize()
	{
		Engine::AssetService* pAssetService = GetEngine()->RequestService<Engine::AssetService>();

		if (pAssetService == nullptr)
			return Engine::ModuleReport("Asset service cannot be reached");

		m_importerContext = GetEngine()->RequestContext<ImporterContext>();

		if (m_importerContext == nullptr)
			return Engine::ModuleReport("Importer context cannot be reached");

		m_projectPath = "D:/Projects/Horizon/ExampleProject";
		m_assetPath = m_projectPath + "/Assets";
		m_cookPath = m_projectPath + "/Cooked";

		if (!PAL::Directory::Exists(m_assetPath) && !PAL::Directory::Create(m_assetPath))
			return Engine::ModuleReport("Asset root cannot be created");

		if (!PAL::Directory::Exists(m_cookPath) && !PAL::Directory::Create(m_cookPath))
			return Engine::ModuleReport("Cook root cannot be created");

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath, m_cookPath);
		RebuildTree();

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

			List<std::string> sources;
			pFolder->Refresh(sources);
			TrackSources(sources);

			++m_revision;
			return;
		}

		TrackSource(event.relativePath);
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

		m_pendingImports.Remove(event.relativePath);

		DomainFolder* pFolder = pParent->FindFolder(event.GetName());

		if (pFolder != nullptr)
			ForgetFolder(pFolder);

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
			m_pendingImports.Remove(event.oldRelativePath);

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

		RebuildTree();
	}

	void DomainService::RebuildTree()
	{
		List<std::string> sources;
		m_root->Refresh(sources);
		TrackSources(sources);

		++m_revision;
	}

	void DomainService::TrackSources(const List<std::string>& relativePaths)
	{
		for (const std::string& relativePath : relativePaths)
			TrackSource(relativePath);
	}

	void DomainService::TrackSource(const std::string& relativePath)
	{
		std::string parentPath;
		std::string name;
		SplitPath(relativePath, parentPath, name);

		DomainFolder* pParent = m_root->ResolveFolder(parentPath);

		if (pParent == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", parentPath);
			return;
		}

		if (pParent->FindFile(name) != nullptr)
			return;

		if (m_pendingImports.Contains(relativePath) || IsImportActive(relativePath))
			return;

		const std::string sourcePath = pParent->GetAbsolutePath() + "/" + name;
		const std::string metaPath = sourcePath + std::string(DomainFile::MetaSuffix);

		if (PAL::File::Exists(metaPath))
		{
			DomainMeta meta;

			if (!meta.Read(metaPath))
			{
				Terminal::Warn(StringOps::GetName(this), "{} has a meta that cannot be loaded, it is left untouched", sourcePath);
				return;
			}

			if (meta.id.IsValid() && !meta.assetTypeName.empty() && PAL::File::Exists(CookedPathOf(meta.id)))
			{
				if (Materialize(pParent, name, meta, false))
					++m_revision;

				return;
			}
		}

		m_pendingImports.PushBack(relativePath);
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
	}

	b8 DomainService::Materialize(DomainFolder* pParent, const std::string& name, const DomainMeta& meta, b8 writeMeta)
	{
		Reflect::Type* pType = GetEngine()->GetReflectionSystem()->GetTypeByName(meta.assetTypeName);

		if (pType == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} names an asset type that is not reflected", name);
			return false;
		}

		DomainFile* pFile = pParent->AddFile(name);
		const b8 hasMeta = writeMeta ? pFile->WriteMetaFile(meta) : pFile->LoadMetaFile();

		if (!hasMeta)
		{
			Terminal::Warn(StringOps::GetName(this), "{} could not get a meta", pFile->GetSourcePath());
			pParent->RemoveEntry(name);
			return false;
		}

		Engine::AssetPhysicalEntry entry = {};
		entry.assetId = pFile->GetID();
		entry.cookPath = pFile->GetCookedPath();
		entry.assetType = pType->GetTypeId();

		auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();

		if (!pAssetService->RegisterAsset(entry))
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be registered", pFile->GetSourcePath());

			if (writeMeta)
				PAL::File::Delete(pFile->GetMetaPath());

			pParent->RemoveEntry(name);
			return false;
		}

		return true;
	}

	std::string DomainService::ResolveAssetTypeName(AssetImporter* pImporter)
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();

		Reflect::Type* pImporterType = pReflect->GetType(pImporter->GetTypeId());

		if (pImporterType == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} importer has no reflected type", StringOps::GetName(pImporter));
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

	std::string DomainService::CookedPathOf(const Guid& id) const
	{
		return m_cookPath + "/" + id.ToString() + std::string(DomainFile::CookSuffix);
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

		const std::string relativePath = m_pendingImports.Front();
		m_pendingImports.PopFront();

		if (IsImportActive(relativePath))
			return;

		std::string parentPath;
		std::string name;
		SplitPath(relativePath, parentPath, name);

		DomainFolder* pParent = m_root->ResolveFolder(parentPath);

		if (pParent == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder vanished before its import could start", parentPath);
			return;
		}

		if (pParent->FindFile(name) != nullptr)
			return;

		const std::string sourcePath = pParent->GetAbsolutePath() + "/" + name;

		if (!PAL::File::Exists(sourcePath))
			return;

		if (!IsSourceReady(sourcePath))
		{
			m_pendingImports.PushBack(relativePath);
			return;
		}

		StartImport(pParent, name, relativePath);
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

			if (pTask->result != AssetImportResult::Success)
				Terminal::Error(StringOps::GetName(this), "{} could not be imported, result code is {}", pTask->sourcePath, static_cast<u32>(pTask->result));
			else if (!pTask->wasCooked)
				Terminal::Error(StringOps::GetName(this), "{} was imported but could not be cooked", pTask->sourcePath);
			else
			{
				std::string parentPath;
				std::string name;
				SplitPath(pTask->relativePath, parentPath, name);

				DomainFolder* pParent = m_root->ResolveFolder(parentPath);

				if (pParent == nullptr || pParent->FindFile(name) != nullptr || !PAL::File::Exists(pTask->sourcePath))
				{
					Terminal::Warn(StringOps::GetName(this), "{} vanished while it was being imported", pTask->sourcePath);
					PAL::File::Delete(pTask->cookedPath);
				}
				else
				{
					DomainMeta meta;
					meta.id = pTask->id;
					meta.assetTypeName = pTask->assetTypeName;

					if (Materialize(pParent, name, meta, true))
						++m_revision;
					else
						PAL::File::Delete(pTask->cookedPath);
				}
			}

			Memory::Allocator::Delete(pTask);
		}
	}

	b8 DomainService::IsImportActive(const std::string& relativePath) const
	{
		for (const ImportTask* pTask : m_activeImports)
		{
			if (pTask->relativePath == relativePath)
				return true;
		}

		return false;
	}

	b8 DomainService::IsSourceReady(const std::string& sourcePath) const
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(sourcePath, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::Exclusive);

		if (!request.IsValid())
			return false;

		PAL::File::ReleaseAccess(request);

		return true;
	}

	b8 DomainService::StartImport(DomainFolder* pParent, const std::string& name, const std::string& relativePath)
	{
		AssetImporter* pImporter = m_importerContext->GetImporter(ExtensionOf(name));

		if (pImporter == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} has no importer, it stays outside the domain", relativePath);
			return false;
		}

		const std::string assetTypeName = ResolveAssetTypeName(pImporter);

		if (assetTypeName.empty())
			return false;

		const std::string sourcePath = pParent->GetAbsolutePath() + "/" + name;
		const std::string metaPath = sourcePath + std::string(DomainFile::MetaSuffix);

		Guid id;

		if (PAL::File::Exists(metaPath))
		{
			DomainMeta meta;

			if (meta.Read(metaPath) && meta.id.IsValid())
				id = meta.id;
		}

		if (!id.IsValid())
			id = Guid::Generate();

		ImportTask* pTask = Memory::Allocator::Create<ImportTask>(Memory::CurrLoc());
		pTask->id = id;
		pTask->relativePath = relativePath;
		pTask->sourcePath = sourcePath;
		pTask->cookedPath = CookedPathOf(id);
		pTask->assetTypeName = assetTypeName;
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

	void DomainService::ReimportAsset(DomainFile* pFile)
	{

	}
}
