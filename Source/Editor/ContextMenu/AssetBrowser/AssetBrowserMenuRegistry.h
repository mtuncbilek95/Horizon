#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <vector>
#include <string>
#include <string_view>

namespace Horizon
{
	class Engine;
	class AssetBrowserMenuItem;
	class AssetBrowserMenuItemAttribute;
	struct AssetBrowserContext;

	class AssetBrowserMenuRegistry
	{
		struct MenuInstance
		{
			AssetBrowserMenuItem* menuItem = nullptr;
			std::string title;
			usize order = 0;

			std::vector<MenuInstance> subMenus;
		};

	public:
		AssetBrowserMenuRegistry(Engine* pEngine);
		~AssetBrowserMenuRegistry();

		void Invalidate();

		void Open();
		void Render(const AssetBrowserContext& context);

	private:
		MenuInstance* EnsurePath(std::string_view path);
		void EnsureCategory(std::string_view path, usize order);
		void InsertMenu(Reflect::Type* pType, const AssetBrowserMenuItemAttribute& attr);
		void SortRecursive(std::vector<MenuInstance>& level);
		void ClearRecursive(MenuInstance& inst);

		void RenderItem(MenuInstance& inst, const AssetBrowserContext& context);

	private:
		Engine* m_engine;

		std::vector<MenuInstance> m_menus;
	};
}