#pragma once

#include <Runtime/Containers/Singleton.h>

namespace Horizon
{
	class Engine;
	class IMenuItem;

	struct MenuTypeInfo
	{
		std::string path;
		usize order;
		IMenuItem* (*CreateMenuItem)(Engine*);
	};

	struct MenuCategoryInfo
	{
		std::string path;
		usize order;
	};

	class MenuFactory : public Singleton<MenuFactory>
	{
	public:
		void RegisterMenu(const MenuTypeInfo& info) { m_types.push_back(info); }
		void RegisterCategory(const MenuCategoryInfo& info) { m_categories.push_back(info); }

		template<typename Fn>
		void ForEachMenu(Fn&& func) const
		{
			for (const auto& info : m_types)
				func(info);
		}

		template<typename Fn>
		void ForEachCategory(Fn&& func) const 
		{
			for (const auto& info : m_categories)
				func(info);
		}

		usize OrderOf(std::string_view path) const
		{
			for (const auto& info : m_categories)
			{
				if (info.path == path)
					return info.order;
			}

			return 100000;
		}

	private:
		std::vector<MenuTypeInfo> m_types;
		std::vector<MenuCategoryInfo> m_categories;
	};
}