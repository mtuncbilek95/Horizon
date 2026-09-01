#pragma once

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	struct H_EXPORT SceneHierarchyContext
	{
		Engine::Engine* pEngine = nullptr;
	};
}