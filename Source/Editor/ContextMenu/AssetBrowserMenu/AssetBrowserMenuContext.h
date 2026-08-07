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
		DomainNode* currentNode;
		const List<DomainNode*>& selected;
		std::filesystem::path* renameRequest;
	};
}