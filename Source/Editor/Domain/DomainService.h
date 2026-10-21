#pragma once

#include <Editor/ImportPipeline/ImporterRegistry.h>
#include <Editor/Domain/DomainFileImportContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Engine/Core/Service.h>
#include <Engine/Job/JobSystem.h>
#include <Runtime/PAL/Watcher/DirectoryWatcher.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Serialization/JsonArchive.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;
	class AssetImporter;

	class EDITOR_API DomainService : public Engine::Service
	{
		static constexpr usize MaxConcurrentImports = 8;
		static constexpr auto IgnoredAddList = std::to_array<std::string_view>({ ".hmeta", ".bin" });

	public:
		Engine::ModuleReport OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

		DomainFolder* GetRoot() const { return m_root; }

		DomainFolder* FindFolder(std::string_view relativePath) const;
		DomainFile* FindFileByGuid(const Guid& guid) const;
		DomainFile* FindFileByRelative(const std::string& relative);

		PAL::DirectoryWatcher& GetWatcher() { return m_watcher; }
		const std::string& GetAssetPath() const { return m_assetPath; }

		u64 GetRevision() const { return m_revision.Load(); }

		Reflect::Type* GetImportSettingsType(const std::string& extension);
		void ImportFile(const std::string& targetFolder, const std::string& sourcePath, const AssetImportSettings* pSettings);

	private:
		void BuildRootTree();
		void BindWatcher();

		void OnEntryAdded(const PAL::DirectoryWatcher::Event& event);
		void OnEntryModified(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRemoved(const PAL::DirectoryWatcher::Event& event);
		void OnEntryRenamed(const PAL::DirectoryWatcher::Event& event);
		void OnWatcherOverflow();

		void RunParseAsset(DomainFileImportContext& task);
		void OnParseAssetFinished(usize slot);

		void RunCookAsset(DomainFileImportContext& task);
		void OnCookAssetFinished(usize slot);

		DomainFile* FindFileByGuid(DomainFolder* pFolder, const Guid& guid) const;
		DomainFile* FindFileByRelative(DomainFolder* pFolder, const std::string& relative) const;
		DomainFile* FindDomainFile(const std::string& relativePathWithExtension) const;

		void ForgetFolder(DomainFolder* pFolder);
		void ForgetFile(DomainFile* pFile);
		void DiscardFile(DomainFile* pFile);

		void ProcessFolder(DomainFolder* pFolder);
		DomainFile* ProcessSourceFile(DomainFolder* pParent, const std::string& name);

		void QueueImport(DomainFile* pFile, ImportReason reason);
		void StartImport();
		void EndImport();

		DomainFileImportContext* FindRunningImport(const Guid& id);
		b8 IsImportPending(const Guid& id) const;

		AssetImportSettings* CloneSettings(const AssetImportSettings* pSource, Reflect::Type* pType);
		b8 ReadSettings(DomainFile* pFile, Reflect::Type* pSettingsType, JsonArchiveReader& reader);

		b8 WriteMetaFile(DomainFile* pFile, Reflect::Type* pSettingsType);
		void ReconcilePath(const std::string& relativePath);
		void ReconcileMeta(const std::string& relativePath);

	private:
		std::string m_assetPath;
		std::string m_cookPath;
		DomainFolder* m_root = nullptr;

		ImporterRegistry m_importPipelineRegistry;
		Engine::JobSystem* m_jobSystem = nullptr;

		PAL::DirectoryWatcher m_watcher;
		PAL::Atomic<u64> m_revision = 0;

		List<DomainFileImportContext> m_pendingImports;
		DomainFileImportContext m_runningImports[MaxConcurrentImports];
	};
}