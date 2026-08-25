#pragma once

#include <Editor/MainMenu/MenuItemInstance.h>
#include <Editor/Renderer/EditorContext.h>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class MenuRegistry
	{
	public:
		MenuRegistry() = default;
		~MenuRegistry();

		void BootstrapMenus(const EditorContext& ctx);
		void RenderGUI();

	private:
		void RenderNode(MenuItemInstance& inst);
		void ClearRecursive(MenuItemInstance& inst);
		void SortRecursive(List<MenuItemInstance>& siblings);

		MenuItemInstance& FindOrCreateContainer(List<MenuItemInstance>& siblings, const std::string& name);

	private:
		List<MenuItemInstance> m_menus;
	};
}