#pragma once

#include <Engine/Plugin/PluginEntry.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Engine
{
	class Engine;

	class ENGINE_API PluginSystem final
	{
		struct PendingRequest
		{
			std::string name;
			b8 load;
		};
	public:
		PluginSystem(Engine* pEngine);
		~PluginSystem();

		PluginSystem(const PluginSystem&) = delete;
		PluginSystem& operator=(const PluginSystem&) = delete;

		const List<PluginEntry>& GetPlugins() const { return m_plugins; }
		const PluginEntry* FindPlugin(const std::string& name) const;

		void SetRoots(const std::string& engineRoot, const std::string& projectRoot);
		void SetEnabledPlugins(const List<std::string>& enabled) { m_enabledPlugins = enabled; }

		void DiscoverPlugins();
		void LoadPlugins();
		void UnloadAll();

		b8 LoadPlugin(const std::string& name);
		b8 UnloadPlugin(const std::string& name);

		void RequestLoad(const std::string& name);
		void RequestUnload(const std::string& name);
		void FlushRequests();

	private:
		PluginEntry* FindEntry(const std::string& name);
		void DiscoverRoot(const std::string& root, PluginOrigin origin, List<PluginEntry>& outEntries);
		b8 LoadEntry(PluginEntry& entry);
		void UnloadEntry(PluginEntry& entry);

	private:
		Engine* m_engine = nullptr;

		std::string m_engineRoot;
		std::string m_projectRoot;
		List<std::string> m_enabledPlugins;
		List<PendingRequest> m_pending;

		List<PluginEntry> m_plugins;
	};
}
