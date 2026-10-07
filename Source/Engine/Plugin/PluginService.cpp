#include "PluginService.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Engine
{
	ModuleReport PluginService::OnInitialize()
	{
		return ModuleReport();
	}

	void PluginService::OnFinalize()
	{
		for (usize i = m_plugins.GetCount(); i > 0; --i)
		{
			UnloadEntry(m_plugins[i - 1]);
		}

		m_plugins.Clear();
	}

	void PluginService::OnExecute(const EngineFrame& ctx)
	{
		FlushRequests();
	}

	void PluginService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Precedes<WindowService>();
	}

	void PluginService::RequestLoad(const std::string& name)
	{
		m_pending.PushBack(PendingRequest{ name, true });
	}

	void PluginService::RequestUnload(const std::string& name)
	{
		m_pending.PushBack(PendingRequest{ name, false });
	}

	void PluginService::FlushRequests()
	{
		if (m_pending.IsEmpty())
		{
			return;
		}

		List<PendingRequest> pending = std::move(m_pending);
		m_pending = {};

		for (const PendingRequest& request : pending)
		{
			if (request.load)
			{
				LoadPlugin(request.name);
			}
			else
			{
				UnloadPlugin(request.name);
			}
		}
	}

	const PluginEntry* PluginService::FindPlugin(const std::string& name) const
	{
		for (const PluginEntry& entry : m_plugins)
		{
			if (entry.name == name)
			{
				return &entry;
			}
		}

		return nullptr;
	}

	void PluginService::SetRoots(const std::string& engineRoot, const std::string& projectRoot)
	{
		m_engineRoot = engineRoot.empty() ? std::string() : StringOps::NormalizePath(engineRoot);
		m_projectRoot = projectRoot.empty() ? std::string() : StringOps::NormalizePath(projectRoot);
	}

	void PluginService::DiscoverPlugins()
	{
		List<PluginEntry> discovered;

		DiscoverRoot(m_engineRoot, PluginOrigin::Engine, discovered);
		DiscoverRoot(m_projectRoot, PluginOrigin::Project, discovered);

		for (PluginEntry& previous : m_plugins)
		{
			if (previous.state != PluginState::Loaded)
			{
				continue;
			}

			b8 replaced = false;

			for (PluginEntry& entry : discovered)
			{
				if (entry.name == previous.name)
				{
					entry = std::move(previous);
					replaced = true;
					break;
				}
			}

			if (!replaced)
			{
				discovered.PushBack(std::move(previous));
			}
		}

		m_plugins = std::move(discovered);

		Terminal::Info(StringOps::GetName(this), "Discovered {} plugins", m_plugins.GetCount());
	}

	b8 PluginService::LoadPlugin(const std::string& name)
	{
		PluginEntry* pEntry = FindEntry(name);

		if (!pEntry)
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a discovered plugin, cannot load", name);
			return false;
		}

		if (pEntry->state == PluginState::Loaded)
		{
			Terminal::Warn(StringOps::GetName(this), "{} is already loaded", name);
			return true;
		}

		return LoadEntry(*pEntry);
	}

	b8 PluginService::UnloadPlugin(const std::string& name)
	{
		PluginEntry* pEntry = FindEntry(name);

		if (!pEntry)
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a discovered plugin, cannot unload", name);
			return false;
		}

		if (pEntry->state != PluginState::Loaded)
		{
			Terminal::Warn(StringOps::GetName(this), "{} is not loaded", name);
			return true;
		}

		UnloadEntry(*pEntry);
		return true;
	}

	PluginEntry* PluginService::FindEntry(const std::string& name)
	{
		for (PluginEntry& entry : m_plugins)
		{
			if (entry.name == name)
			{
				return &entry;
			}
		}

		return nullptr;
	}

	void PluginService::DiscoverRoot(const std::string& root, PluginOrigin origin, List<PluginEntry>& outEntries)
	{
		if (root.empty() || !PAL::Directory::Exists(root))
		{
			return;
		}

		for (const PAL::Directory::Entry& dirEntry : PAL::Directory::Iterate(root))
		{
			if (!dirEntry.isDirectory)
			{
				continue;
			}

			const std::string libraryPath = dirEntry.fullPath + "/" + dirEntry.name + ".dll";

			if (!PAL::File::Exists(libraryPath))
			{
				continue;
			}

			PluginEntry* pExisting = nullptr;

			for (PluginEntry& entry : outEntries)
			{
				if (entry.name == dirEntry.name)
				{
					pExisting = &entry;
					break;
				}
			}

			PluginEntry& entry = pExisting ? *pExisting : outEntries.EmplaceBack();
			entry.name = dirEntry.name;
			entry.directory = dirEntry.fullPath;
			entry.libraryPath = libraryPath;
			entry.origin = origin;
			entry.state = PluginState::Available;
		}
	}

	b8 PluginService::LoadEntry(PluginEntry& entry)
	{
		PAL::SymbolLibraryDesc desc;
		desc.path = entry.libraryPath;
		desc.isMain = false;

		PAL::SymbolLibrary* pLibrary = Memory::Allocator::Create<PAL::SymbolLibrary>(Memory::CurrLoc(), desc);

		if (!pLibrary || pLibrary->GetName().empty())
		{
			Memory::Allocator::Delete(pLibrary);
			entry.state = PluginState::Failed;
			entry.failReason = "Library could not be loaded";
			Terminal::Error(StringOps::GetName(this), "{} could not be loaded from {}", entry.name, entry.libraryPath);
			return false;
		}

		if (!GetEngine()->RegisterLibrary(pLibrary))
		{
			Memory::Allocator::Delete(pLibrary);
			entry.state = PluginState::Failed;
			entry.failReason = "Manifestation could not be registered";
			Terminal::Error(StringOps::GetName(this), "{} loaded but its manifestation could not be registered", entry.name);
			return false;
		}

		entry.pLibrary = pLibrary;
		entry.state = PluginState::Loaded;
		entry.failReason.clear();

		Terminal::Info(StringOps::GetName(this), "{} loaded", entry.name);
		return true;
	}

	void PluginService::UnloadEntry(PluginEntry& entry)
	{
		if (!entry.pLibrary)
		{
			return;
		}

		GetEngine()->UnregisterLibrary(entry.pLibrary);
		Memory::Allocator::Delete(entry.pLibrary);
		entry.pLibrary = nullptr;
		entry.state = PluginState::Available;

		Terminal::Info(StringOps::GetName(this), "{} unloaded", entry.name);
	}

	void PluginService::LoadPlugins()
	{
		for (const std::string& name : m_enabledPlugins)
		{
			if (!LoadPlugin(name))
				Terminal::Error(StringOps::GetName(this), "{} could not loaded", name);
		}
	}
}