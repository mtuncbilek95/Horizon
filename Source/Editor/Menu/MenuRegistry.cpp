#include "MenuRegistry.h"

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <imgui.h>
#include <algorithm>

namespace Horizon
{
	namespace
	{
		constexpr usize UndefinedOrder = 100000;
	}

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
		for (MenuInstance& menu : m_menus)
			ClearRecursive(menu);

		m_menus.clear();

		ModuleContext* modCtx = m_engine->GetModuleContext();

		// Containers
		for (TypeManifest* manifest : modCtx->GetManifestsByAttribute(TypeIdOf<MainMenuAttribute>()))
		{
			for (MainMenuAttribute* attr : manifest->GetCustomAttributes<MainMenuAttribute>())
				EnsureCategory(attr->GetPath(), attr->GetOrder());
		}

		// Leafs
		for (TypeManifest* manifest : modCtx->GetManifestsByBase(TypeIdOf<IMenuItem>()))
		{
			MenuItemAttribute* attr = manifest->GetCustomAttribute<MenuItemAttribute>();
			if (!attr)
			{
				Terminal::Warn("MenuRegistry", "IMenuItem manifest without MenuItemAttribute, skipping");
				continue;
			}

			InsertMenu(manifest, *attr);
		}

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

	MenuRegistry::MenuInstance* MenuRegistry::EnsurePath(std::string_view path)
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
			for (MenuInstance& candidate : *level)
			{
				if (candidate.title == segment)
				{
					node = &candidate;
					break;
				}
			}

			if (!node)
			{
				level->push_back(MenuInstance{ nullptr, segment, UndefinedOrder, {} });
				node = &level->back();
			}

			if (last)
				break;

			level = &node->subMenus;
			start = slash + 1;
		}

		return node;
	}

	void MenuRegistry::EnsureCategory(std::string_view path, usize order)
	{
		MenuInstance* node = EnsurePath(path);
		if (node)
			node->order = order;
	}

	void MenuRegistry::InsertMenu(TypeManifest* manifest, const MenuItemAttribute& attr)
	{
		MenuInstance* node = EnsurePath(attr.GetPath());
		if (!node)
			return;

		IMenuItem* item = static_cast<IMenuItem*>(manifest->Create());
		if (!item)
		{
			Terminal::Warn("MenuRegistry", "manifest->Create() returned null for menu item");
			return;
		}

		item->SetEngine(m_engine);

		node->menuItem = item;
		node->order = attr.GetOrder();
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