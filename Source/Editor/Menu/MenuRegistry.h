#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <vector>
#include <string>
#include <string_view>

namespace Horizon
{
	class Engine;
	class IMenuItem;
	class Type;
	class MenuItemAttribute;

	class MenuRegistry
	{
		struct MenuInstance
		{
			IMenuItem* menuItem = nullptr;
			std::string title;
			usize order = 0;

			std::vector<MenuInstance> subMenus;
		};


	public:
		MenuRegistry(Engine* pEngine);
		~MenuRegistry();

		void Invalidate();
		void Render();

	private:
		MenuInstance* EnsurePath(std::string_view path);
		void EnsureCategory(std::string_view path, usize order);
		void InsertMenu(Reflect::Type* pType, const MenuItemAttribute& attr);
		void SortRecursive(std::vector<MenuInstance>& level);
		void ClearRecursive(MenuInstance& inst);

		void RenderItem(MenuInstance& inst);

	private:
		Engine* m_engine;

		std::vector<MenuInstance> m_menus;
	};
}