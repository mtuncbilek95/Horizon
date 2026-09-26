#pragma once

#include <Editor/Importer/AssetImportTask.h>
#include <Engine/Core/Service.h>
#include <Runtime/PAL/Watcher/DirectoryWatcher.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;
	struct DomainMeta;
	class AssetImporter;
	class ImporterContext;

	class H_EXPORT DomainService : public Engine::Service
	{
		static constexpr usize MaxConcurrentImports = 4;
	public:
		static void RunImport(ImportTask* pTask);

	public:
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

		void ReimportAsset(DomainFile* pFile);

	private:
		static void SplitPath(std::string_view relativePath, std::string& outParent, std::string& outName);
		static std::string ExtensionOf(const std::string& name);

		void BindWatcher();

		void OnEntryAdded(const PAL::DirectoryWatcher::Event& event);
		void OnEntryModified(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRemoved(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRenamed(const PAL::DirectoryWatcher::Event& event);
		void OnWatcherOverflow();

		DomainFile* FindFileByGuid(DomainFolder* pFolder, const Guid& guid) const;

		void RebuildTree();
		void TrackSources(const List<std::string>& relativePaths);
		void TrackSource(const std::string& relativePath);
		void ForgetFolder(DomainFolder* pFolder);
		void ForgetFile(DomainFile* pFile);

		b8 Materialize(DomainFolder* pParent, const std::string& name, const DomainMeta& meta, b8 writeMeta);
		std::string ResolveAssetTypeName(AssetImporter* pImporter);
		std::string CookedPathOf(const Guid& id) const;
		void MoveMeta(const std::string& oldMetaPath, const std::string& newMetaPath);

		void ProcessPendingImports();
		void CommitFinishedImports();
		b8 IsImportActive(const std::string& relativePath) const;
		b8 IsSourceReady(const std::string& sourcePath) const;
		b8 StartImport(DomainFolder* pParent, const std::string& name, const std::string& relativePath);

	private:
		std::string m_projectPath;
		std::string m_assetPath;
		std::string m_cookPath;
		DomainFolder* m_root = nullptr;

		ImporterContext* m_importerContext = nullptr;

		List<std::string> m_pendingImports;
		List<ImportTask*> m_activeImports;

		PAL::DirectoryWatcher m_watcher;
		b8 m_watcherHealthy = false;
		u64 m_revision = 0;
	};
}
