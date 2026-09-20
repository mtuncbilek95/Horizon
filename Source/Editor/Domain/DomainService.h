#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Asset/Sources/LooseSourceFile.h>
#include <Runtime/PAL/Watcher/DirectoryWatcher.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;
	class ImporterContext;

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
		void OnEntryModified(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRemoved(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRenamed(const PAL::DirectoryWatcher::Event& event);
		void OnWatcherOverflow();

		DomainFile* FindFileByGuid(DomainFolder* pFolder, const Guid& guid) const;

		void TrackFolder(DomainFolder* pFolder);
		void TrackFile(DomainFile* pFile);
		void ForgetFile(DomainFile* pFile);

		b8 EnsureMeta(DomainFile* pFile);
		std::string ResolveAssetTypeName(DomainFile* pFile);
		void MoveMeta(const std::string& oldMetaPath, const std::string& newMetaPath);

		void QueueImport(const Guid& id);
		void ProcessPendingImports();
		b8 IsSourceReady(DomainFile* pFile) const;
		b8 ImportFile(DomainFile* pFile);
		b8 RegisterCooked(DomainFile* pFile);

	private:
		std::string m_projectPath;
		std::string m_assetPath;
		std::string m_cookPath;
		DomainFolder* m_root = nullptr;

		Engine::LooseSourceFile* m_projectSource = nullptr;
		ImporterContext* m_importerContext = nullptr;

		List<Guid> m_pendingImports;

		PAL::DirectoryWatcher m_watcher;
		b8 m_watcherHealthy = false;
		u64 m_revision = 0;
	};
}