#pragma once

#include <Editor/Menu/MenuFactory.h>

#include <vector>

namespace Horizon
{
	class Engine;
	class IMenuItem;

	class MenuRegistry
	{
		struct MenuInstance
		{
			IMenuItem* menuItem;
			std::string title;
			usize order;
			
			std::vector<MenuInstance> subMenus;
		};

	public:
		MenuRegistry(Engine* pEngine);
		~MenuRegistry();

		void Invalidate();
		void Render();

	private:
		MenuInstance* EnsurePath(std::string_view path, b8 terminalIsContainer);
		void EnsureCategory(const MenuCategoryInfo& cat);
		void InsertMenu(const MenuTypeInfo& info);
		void SortRecursive(std::vector<MenuInstance>& level);
		void ClearRecursive(MenuInstance& inst);

		void RenderItem(MenuInstance& inst);

	private:
		Engine* m_engine;

		std::vector<MenuInstance> m_menus;
	};
}