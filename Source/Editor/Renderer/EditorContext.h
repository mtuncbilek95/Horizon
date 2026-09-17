#pragma once

#include <Editor/Models/SelectionModel.h>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	struct H_EXPORT EditorContext final
	{
		Engine::Engine* pEngine = nullptr;
		SelectionModel* pSelection = nullptr;
	};
}