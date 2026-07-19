#include "AssetBrowserMenuRegistry.h"

#include <Editor/ContextMenu/AssetBrowser/AssetBrowserMenuItem.h>
#include <Editor/ContextMenu/AssetBrowser/AssetBrowserMenuAttribute.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserContext.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <algorithm>

namespace Horizon
{
	namespace
	{
		constexpr usize UndefinedOrder = 100000;
		constexpr const char* PopupId = "AssetBrowserContextMenu";
	}

	AssetBrowserMenuRegistry::AssetBrowserMenuRegistry(Engine* pEngine) : m_engine(pEngine)
	{
	}

	AssetBrowserMenuRegistry::~AssetBrowserMenuRegistry()
	{
		for (auto& menu : m_menus)
			ClearRecursive(menu);
	}

	void AssetBrowserMenuRegistry::Invalidate()
	{
		for (MenuInstance& menu : m_menus)
			ClearRecursive(menu);

		m_menus.clear();

		ModuleContext* modCtx = m_engine->GetModuleContext();

		// Categories (embedded)
		for (Reflect::Type* pType : modCtx->GetTypeByAttribute(Reflect::TypeOf<AssetBrowserMainAttribute>()))
		{
			for (AssetBrowserMainAttribute* attr : pType->GetCustomAttributes<AssetBrowserMainAttribute>())
				EnsureCategory(attr->GetPath(), attr->GetOrder());
		}

		// Leafs
		for (Reflect::Type* pType : modCtx->GetTypeByBase(Reflect::TypeOf<AssetBrowserMenuItem>()))
		{
			AssetBrowserMenuItemAttribute* attr = pType->GetCustomAttribute<AssetBrowserMenuItemAttribute>();
			if (!attr)
			{
				Terminal::Warn("AssetBrowserMenuRegistry", "AssetBrowserMenuItem manifest without AssetBrowserMenuItemAttribute, skipping");
				continue;
			}

			InsertMenu(pType, *attr);
		}

		SortRecursive(m_menus);
	}

	void AssetBrowserMenuRegistry::Open()
	{
		ImGui::OpenPopup(PopupId);
	}

	void AssetBrowserMenuRegistry::Render(const AssetBrowserContext& context)
	{
		if (ImGui::BeginPopup(PopupId))
		{
			for (auto& menu : m_menus)
				RenderItem(menu, context);

			ImGui::EndPopup();
		}
	}

	AssetBrowserMenuRegistry::MenuInstance* AssetBrowserMenuRegistry::EnsurePath(std::string_view path)
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

	void AssetBrowserMenuRegistry::EnsureCategory(std::string_view path, usize order)
	{
		auto* node = EnsurePath(path);
		if (node)
			node->order = order;
	}

	void AssetBrowserMenuRegistry::InsertMenu(Reflect::Type* pType, const AssetBrowserMenuItemAttribute& attr)
	{
		auto* node = EnsurePath(attr.GetPath());
		if (!node)
			return;

		auto* item = static_cast<AssetBrowserMenuItem*>(pType->CreateFromMemory());
		if (!item)
		{
			Terminal::Warn("AssetBrowserMenuRegistry", "CreateFromMemory returned null for menu item");
			return;
		}

		item->SetEngine(m_engine);

		node->menuItem = item;
		node->order = attr.GetOrder();
	}

	void AssetBrowserMenuRegistry::SortRecursive(std::vector<MenuInstance>& level)
	{
		std::sort(level.begin(), level.end(), [](const MenuInstance& a, const MenuInstance& b)
			{
				return a.order < b.order;
			});

		for (auto& node : level)
			SortRecursive(node.subMenus);
	}

	void AssetBrowserMenuRegistry::ClearRecursive(MenuInstance& inst)
	{
		if (inst.menuItem)
			Allocator::Delete(inst.menuItem);

		for (auto& menu : inst.subMenus)
			ClearRecursive(menu);

		inst.subMenus.clear();
	}

	void AssetBrowserMenuRegistry::RenderItem(MenuInstance& inst, const AssetBrowserContext& context)
	{
		if (!inst.menuItem)
		{
			if (ImGui::BeginMenu(inst.title.c_str(), !inst.subMenus.empty()))
			{
				for (auto& sub : inst.subMenus)
					RenderItem(sub, context);

				ImGui::EndMenu();
			}

			return;
		}

		if (ImGui::MenuItem(inst.title.c_str()))
			inst.menuItem->OnInvoke(context);
	}
}