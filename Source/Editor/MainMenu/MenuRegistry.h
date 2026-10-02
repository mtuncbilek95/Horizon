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
		static constexpr f32 kHeight = 32.0f;
		static constexpr f32 kButtonWidth = 46.0f;
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
		PAL::ChromeRect RenderChromeButton(const c8* pLabel, b8 danger);

	private:
		PAL::Window* m_window = nullptr;
		List<MenuItemInstance> m_menus;
	};
}