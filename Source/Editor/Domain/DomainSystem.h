#pragma once

#include <Engine/Core/System.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Timer/Timer.h>
#include <Runtime/PAL/Watcher/DirectoryWatcher.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;

	class H_EXPORT DomainSystem : public Engine::System
	{
		static constexpr std::string_view MetaFileExt = ".hmeta";

		static constexpr f64 DebounceSeconds = 0.3;
		static constexpr f64 FallbackScanSeconds = 0.5;
		static constexpr f64 SafetyScanSeconds = 30.0;

		static constexpr usize MaxFolderSyncPerFrame = 4;
		static constexpr usize MaxImportsPerFrame = 2;

	public:
		DomainSystem(const std::string& projectPath);
		~DomainSystem();

		Engine::AppReport OnAttach(Engine::Application* pEngine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(Engine::OrderRules& rules) const final;
		void GetExecutionOrder(Engine::OrderRules& rules) const final;

		DomainFolder* GetRootFolder() const { return m_root; }
		const std::string& GetAssetPath() const { return m_assetPath; }

		void RequestFullRescan() { m_fullRescan = true; }

	private:
		void DrainWatcher(f64 now);
		void ProcessDirtyFolders(f64 now);
		void ProcessPendingImports(f64 now);
		void RunFullRescan(f64 now);

		void SyncFolder(DomainFolder* pTarget, b8 recursive);

		void MarkDirty(const std::string& absolutePath, f64 now);
		void MarkImport(const std::string& absolutePath, f64 now);

		DomainFolder* ResolveFolder(const std::string& absolutePath, b8& outExact) const;
		DomainFolder* FindFolder(const List<DomainFolder*>& folders, const std::string& folderName) const;

	private:
		std::string m_projectPath;
		std::string m_assetPath;
		DomainFolder* m_root = nullptr;

		PAL::DirectoryWatcher m_watcher;
		List<PAL::DirectoryWatcher::Event> m_events;
		b8 m_watcherHealthy = false;

		std::unordered_map<std::string, f64> m_dirtyFolders;
		std::unordered_map<std::string, f64> m_pendingImports;

		b8 m_fullRescan = false;
		f64 m_lastFullScan = 0.0;

		PAL::Timer m_clock;
	};
}