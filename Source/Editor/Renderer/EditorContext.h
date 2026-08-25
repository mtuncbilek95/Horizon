#pragma once

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class SelectionModel;

	struct H_EXPORT EditorContext final
	{
		Engine::Engine* pEngine = nullptr;
		SelectionModel* pSelection = nullptr;
	};
}