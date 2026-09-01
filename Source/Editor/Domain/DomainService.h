#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Watcher/DirectoryWatcher.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;

	class H_EXPORT DomainService : public Engine::Service
	{
	public:
		DomainService();
		~DomainService();

		Engine::ModuleReport OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx) final;
		void OnFinalize() final;

		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		DomainFolder* GetRoot() const { return m_root; }
		DomainFolder* FindFolder(std::string_view relativePath) const;
		DomainFile* FindFileByGuid(const Guid& guid) const;

		PAL::DirectoryWatcher& GetWatcher() { return m_watcher; }

		const std::string& GetProjectPath() const { return m_projectPath; }
		const std::string& GetAssetPath() const { return m_assetPath; }

		u64 GetRevision() const { return m_revision; }

	private:
		void BindWatcher();

		void OnEntryAdded(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRemoved(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRenamed(const PAL::DirectoryWatcher::Event& event);
		void OnWatcherOverflow();

		DomainFile* FindFileByGuid(DomainFolder* pFolder, const Guid& guid) const;

		std::string m_projectPath;
		std::string m_assetPath;

		DomainFolder* m_root = nullptr;

		PAL::DirectoryWatcher m_watcher;
		b8 m_watcherHealthy = false;
		u64 m_revision = 0;
	};
}