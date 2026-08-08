#pragma once

#include <Editor/MainMenu/MenuItemInstance.h>

namespace Horizon::Engine
{
	class Application;
}

namespace Horizon::Editor
{
	class MenuRegistry
	{
	public:
		MenuRegistry() = default;
		~MenuRegistry();

		void BootstrapMenus(Engine::Application* pEngine);
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