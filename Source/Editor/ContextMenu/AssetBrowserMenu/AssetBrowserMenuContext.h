#pragma once

#include <Runtime/Containers/List.h>

#include <filesystem>

namespace Horizon
{
	class Engine;
	class DomainNode;

	struct AssetBrowserMenuContext
	{
		Engine* engine;
		const List<DomainNode*>& selected;
		std::filesystem::path* renameRequest;
	};
}