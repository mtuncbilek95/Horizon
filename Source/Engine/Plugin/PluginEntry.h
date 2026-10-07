#pragma once

#include <Runtime/PAL/Module/SymbolLibrary.h>

#include <string>

namespace Horizon::Engine
{
	enum class PluginOrigin : u8
	{
		Engine,
		Project
	};

	enum class PluginState : u8
	{
		Available,
		Loaded,
		Failed
	};

	struct PluginEntry
	{
		std::string name;
		std::string directory;
		std::string libraryPath;
		std::string failReason;

		PluginOrigin origin = PluginOrigin::Project;
		PluginState state = PluginState::Available;

		PAL::SymbolLibrary* pLibrary = nullptr;
	};
}