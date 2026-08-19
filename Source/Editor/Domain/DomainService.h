// DomainService.h

#pragma once

#include <Editor/Domain/DomainScanResult.h>
#include <Engine/Core/Service.h>
#include <Engine/Job/Job.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/PAL/Sync/Mutex.h>
#include <Runtime/PAL/Timer/Timer.h>
#include <Runtime/PAL/Watcher/DirectoryWatcher.h>

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;

	class H_EXPORT DomainService : public Engine::Service
	{
		static constexpr std::string_view MetaSuffix = ".hmeta";
		static constexpr std::string_view MetaExtension = "hmeta";

		static constexpr f64 DebounceSeconds = 0.3;
		static constexpr f64 SafetyScanSeconds = 30.0;
		static constexpr f64 FallbackScanSeconds = 0.5;

		struct DirtyFolder
		{
			f64 timestamp = 0.0;
			b8 recursive = false;
		};

		struct PathHash
		{
			using is_transparent = void;

			usize operator()(std::string_view value) const { return std::hash<std::string_view>{}(value); }
		};

		using FolderIndex = std::unordered_map<std::string, DomainFolder*, PathHash, std::equal_to<>>;

	public:
		DomainService();
		~DomainService();

		Engine::ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;

		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		DomainFolder* GetRoot() const { return m_root; }
		DomainFolder* FindFolder(std::string_view relativePath) const;

		const std::string& GetProjectPath() const { return m_projectPath; }
		const std::string& GetAssetPath() const { return m_assetPath; }

		u64 GetRevision() const { return m_revision; }
		void RequestFullRescan() { m_rescanRequest.Store(1); }

	private:
		void SubmitScan();
		void DrainResults();
		void ApplyFolderSnapshot(const DomainFolderSnapshot& snapshot);
		void DestroyFolder(DomainFolder* pFolder, const std::string& relativePath);

		void RunScan();
		void CollectDirty(f64 now);
		void MarkDirty(std::string_view relativePath, f64 now, b8 recursive);
		void ScanFolder(const std::string& relativePath, b8 recursive, DomainScanResult& outResult);

		std::string ToAbsolute(std::string_view relativePath) const;

	private:
		std::string m_projectPath;
		std::string m_assetPath;

		DomainFolder* m_root = nullptr;
		FolderIndex m_folderIndex;
		u64 m_revision = 0;

		PAL::DirectoryWatcher m_watcher;
		List<PAL::DirectoryWatcher::Event> m_events;
		PAL::Timer m_clock;
		std::unordered_map<std::string, DirtyFolder> m_dirtyFolders;
		b8 m_watcherHealthy = false;
		b8 m_pendingFullRescan = true;
		f64 m_lastFullScan = 0.0;

		PAL::Atomic<u32> m_scanBusy = 0;
		PAL::Atomic<u32> m_rescanRequest = 0;
		Engine::JobCounter m_scanCounter;
		PAL::Mutex m_resultLock;
		List<DomainScanResult> m_pendingResults;
	};
}