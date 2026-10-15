#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/DomainMeta.h>
#include <Editor/Project/ProjectContext.h>

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

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath, m_cookPath);

		m_watcher = PAL::DirectoryWatcher(m_assetPath, true);
		BindWatcher();

		m_importPipelineRegistry.BootstrapViews(GetEngine());
		return Engine::ModuleReport();
	}

	void DomainService::OnExecute(const Engine::EngineFrame& ctx)
	{
		if(m_watcher.IsValid())
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

		// Create the hmeta with ImportPipelineSettings

		// Cook the asset file. If it's a success ? add to the domain tree : remove everything created.
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
}