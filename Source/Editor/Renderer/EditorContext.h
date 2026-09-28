#pragma once

#include <Editor/Models/SelectionModel.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class MenuRegistry;
	class ViewRegistry;
	class ToolBarRegistry;

	struct H_EXPORT EditorContext final
	{
		PAL::Window* pWindow = nullptr;
		Engine::Engine* pEngine = nullptr;
		SelectionModel* pSelection = nullptr;

		ViewRegistry* pViews = nullptr;
		MenuRegistry* pMenus = nullptr;
		ToolBarRegistry* pTools = nullptr;
	};
}