#include "MenuRegistry.h"

#include <Editor/Menu/IMenuItem.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>

namespace Horizon
{
	MenuRegistry::MenuRegistry(Engine* pEngine) : m_engine(pEngine)
	{
	}

	MenuRegistry::~MenuRegistry()
	{
		for (auto& menu : m_menus)
			ClearRecursive(menu);
	}

	void MenuRegistry::Invalidate()
	{
		for (auto& menu : m_menus)
			ClearRecursive(menu);

		m_menus.clear();

		MenuFactory::Get().ForEachCategory([this](const MenuCategoryInfo& cat)
			{
				EnsureCategory(cat);
			});

		MenuFactory::Get().ForEachMenu([this](const MenuTypeInfo& info)
			{
				InsertMenu(info);
			});

		SortRecursive(m_menus);
	}

	void MenuRegistry::Render()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 2.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(6.0f, 4.0f));

		if (ImGui::BeginMainMenuBar())
		{
			for (auto& menu : m_menus)
				RenderItem(menu);

			ImGui::EndMainMenuBar();
		}

		ImGui::PopStyleVar(3);
	}

	MenuRegistry::MenuInstance* MenuRegistry::EnsurePath(std::string_view path, b8 terminalIsContainer)
	{
		std::vector<MenuInstance>* level = &m_menus;
		MenuInstance* node = nullptr;

		usize start = 0;
		while (start <= path.size())
		{
			usize slash = path.find('/', start);
			b8 last = (slash == std::string_view::npos);
			usize end = last ? path.size() : slash;

			std::string segment(path.substr(start, end - start));

			node = nullptr;
			for (auto& candidate : *level)
			{
				if (candidate.title == segment)
				{
					node = &candidate;
					break;
				}
			}

			if (!node)
			{
				b8 isContainer = (!last || terminalIsContainer);
				usize order = isContainer ? MenuFactory::Get().OrderOf(path.substr(0, end)) : 0;

				level->push_back(MenuInstance{ nullptr, segment, order, {} });
				node = &level->back();
			}

			if (last)
				break;

			level = &node->subMenus;
			start = slash + 1;
		}

		return node;
	}

	void MenuRegistry::EnsureCategory(const MenuCategoryInfo& cat)
	{
		auto* node = EnsurePath(cat.path, true);
		node->order = cat.order;
	}

	void MenuRegistry::InsertMenu(const MenuTypeInfo& info)
	{
		auto* node = EnsurePath(info.path, false);

		IMenuItem* pItem = info.CreateMenuItem(m_engine);
		if (!pItem)
		{
			Terminal::Warn("MenuRegistry", "Factory returned null for a menu item. Skipping.");
			return;
		}

		node->menuItem = pItem;
		node->order = info.order;
	}

	void MenuRegistry::SortRecursive(std::vector<MenuInstance>& level)
	{
		std::sort(level.begin(), level.end(), [](const MenuInstance& a, const MenuInstance& b)
			{
				return a.order < b.order;
			});

		for (auto& node : level)
			SortRecursive(node.subMenus);
	}

	void MenuRegistry::ClearRecursive(MenuInstance& inst)
	{
		if (inst.menuItem)
			Allocator::Delete(inst.menuItem);

		for (auto& menu : inst.subMenus)
			ClearRecursive(menu);

		inst.subMenus.clear();
	}

	void MenuRegistry::RenderItem(MenuInstance& inst)
	{
		if (!inst.menuItem)
		{
			if (ImGui::BeginMenu(inst.title.c_str(), !inst.subMenus.empty()))
			{
				for (auto& sub : inst.subMenus)
					RenderItem(sub);

				ImGui::EndMenu();
			}

			return;
		}

		if (ImGui::MenuItem(inst.title.c_str()))
			inst.menuItem->OnInvoke();
	}

}