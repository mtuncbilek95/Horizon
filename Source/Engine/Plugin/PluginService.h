#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Plugin/PluginEntry.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Engine
{
	class ENGINE_API PluginService final : public Service
	{
	public:
		PluginService() = default;
		~PluginService() = default;

		PluginService(const PluginService&) = delete;
		PluginService& operator=(const PluginService&) = delete;

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final {}
		void OnLibraryUnregistered(const ReflectionLibrary& library) final {}

		const List<PluginEntry>& GetPlugins() const { return m_plugins; }
		const PluginEntry* FindPlugin(const std::string& name) const;

		void SetRoots(const std::string& engineRoot, const std::string& projectRoot);
		void SetEnabledPlugins(const List<std::string>& enabled) { m_enabledPlugins = enabled; }

		void Discover();
		void Load();
		b8 LoadPlugin(const std::string& name);
		b8 UnloadPlugin(const std::string& name);

		void RequestLoad(const std::string& name);
		void RequestUnload(const std::string& name);

	private:
		struct PendingRequest
		{
			std::string name;
			b8 load;
		};

		void FlushRequests();
		PluginEntry* FindEntry(const std::string& name);
		void DiscoverRoot(const std::string& root, PluginOrigin origin, List<PluginEntry>& outEntries);
		b8 LoadEntry(PluginEntry& entry);
		void UnloadEntry(PluginEntry& entry);

	private:
		std::string m_engineRoot;
		std::string m_projectRoot;
		List<std::string> m_enabledPlugins;
		List<PendingRequest> m_pending;

		List<PluginEntry> m_plugins;
	};
}
