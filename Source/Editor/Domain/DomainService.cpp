#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/ModuleGraph.h>
#include <Engine/Asset/AssetService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>

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
		m_projectPath = "D:/Projects/Horizon/ExampleProject";
		m_assetPath = m_projectPath + "/Assets";

		if (!PAL::Directory::Exists(m_assetPath) && !PAL::Directory::Create(m_assetPath))
			return Engine::ModuleReport("Asset root cannot be created");

		m_root = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), nullptr, "Assets", m_assetPath);
		m_root->Refresh();

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

	void DomainService::OnExecute()
	{
		if (!m_watcherHealthy)
			return;

		if (m_watcher.Dispatch())
			return;

		Terminal::Error(StringOps::GetName(this), "{} watcher stopped, the domain tree will not refresh", m_assetPath);
		m_watcherHealthy = false;
	}

	void DomainService::OnFinalize()
	{
		m_watcher.ClearSubscriptions();
		m_watcher = PAL::DirectoryWatcher();
		m_watcherHealthy = false;

		if (m_root == nullptr)
			return;

		Memory::Allocator::Delete(m_root);
		m_root = nullptr;
	}

	void DomainService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Precedes<Engine::AssetService>();
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
		if (event.GetExtension() == DomainFile::MetaExtension)
			return;

		DomainFolder* pParent = m_root->ResolveFolder(event.GetParent());

		if (pParent == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", event.GetParent());
			return;
		}

		if (event.kind == PAL::WatcherEntryKind::Directory)
		{
			pParent->AddFolder(std::string(event.GetName()))->Refresh();
			++m_revision;
			return;
		}

		if (pParent->AddFile(std::string(event.GetName())) == nullptr)
		{
			Terminal::Warn(StringOps::GetName(this), "{} could not be tracked", event.relativePath);
			return;
		}

		++m_revision;
	}

	void DomainService::OnEntryRemoved(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetExtension() == DomainFile::MetaExtension)
			return;

		DomainFolder* pParent = m_root->ResolveFolder(event.GetParent());

		if (pParent == nullptr)
		{
			Terminal::Debug(StringOps::GetName(this), "{} folder is not tracked", event.GetParent());
			return;
		}

		if (!pParent->RemoveEntry(event.GetName()))
			return;

		++m_revision;
	}

	void DomainService::OnEntryRenamed(const PAL::DirectoryWatcher::Event& event)
	{
		if (event.GetExtension() == DomainFile::MetaExtension)
			return;

		DomainFolder* pOldParent = m_root->ResolveFolder(event.GetOldParent());

		if (pOldParent != nullptr)
			pOldParent->RemoveEntry(event.GetOldName());

		OnEntryAdded(event);
	}

	void DomainService::OnWatcherOverflow()
	{
		Terminal::Warn(StringOps::GetName(this), "{} overflowed its notifications, the domain tree is rebuilt", m_assetPath);

		m_root->Refresh();
		++m_revision;
	}
}