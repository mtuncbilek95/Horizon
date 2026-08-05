#include "MenuRegistry.h"

#include <Editor/Attributes/MainMenuItemAttribute.h>
#include <Editor/Attributes/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <imgui.h>

namespace Horizon
{
	MenuRegistry::~MenuRegistry()
	{
		for (auto& inst : m_menus)
			ClearRecursive(inst);
		m_menus.Clear();
	}

	void MenuRegistry::BootstrapMenus(Engine* pEngine)
	{
		for (auto& inst : m_menus)
			ClearRecursive(inst);
		m_menus.Clear();

		auto* moduleCtx = pEngine->GetModuleContext();

		List<Reflect::Type*> mainList = moduleCtx->GetTypeByAttribute(Reflect::TypeOf<MainMenuItemAttribute>());

		for (auto* type : mainList)
		{
			auto* attr = type->GetCustomAttribute<MainMenuItemAttribute>();

			MenuItemInstance inst;
			inst.displayName = attr->GetPath();
			inst.isCheckbox = false;
			inst.order = attr->GetOrder();
			inst.menu = nullptr;

			m_menus.PushBack(std::move(inst));
		}

		List<Reflect::Type*> leafList = moduleCtx->GetTypeByAttribute(Reflect::TypeOf<MenuItemAttribute>());

		for (auto* type : leafList)
		{
			auto* attr = type->GetCustomAttribute<MenuItemAttribute>();
			const std::string& path = attr->GetPath();
			i32 order = attr->GetOrder();

			List<MenuItemInstance>* level = &m_menus;
			usize start = 0;
			b8 rootSegment = true;

			while (true)
			{
				const usize slash = path.find('/', start);
				const b8 isLeaf = (slash == std::string::npos);
				std::string segment = path.substr(start, isLeaf ? std::string::npos : slash - start);

				if (isLeaf)
				{
					auto* menuObj = static_cast<MenuItem*>(type->CreateFromMemory());
					menuObj->m_engine = pEngine;

					MenuItemInstance leaf;
					leaf.displayName = std::move(segment);
					leaf.isCheckbox = attr->GetIsCheckbox();
					leaf.order = order;
					leaf.menu = menuObj;

					level->PushBack(std::move(leaf));
					break;
				}

				MenuItemInstance& container = FindOrCreateContainer(*level, segment);

				if (!rootSegment && order < container.order)
					container.order = order;

				level = &container.subMenus;
				start = slash + 1;
				rootSegment = false;
			}
		}

		SortRecursive(m_menus);
	}

	void MenuRegistry::RenderGUI()
	{
		if (ImGui::BeginMainMenuBar())
		{
			for (auto& root : m_menus)
				RenderNode(root);

			ImGui::EndMainMenuBar();
		}
	}

	void MenuRegistry::RenderNode(MenuItemInstance& inst)
	{
		if (inst.menu == nullptr)
		{
			if (ImGui::BeginMenu(inst.displayName.c_str(), !inst.subMenus.IsEmpty()))
			{
				for (auto& child : inst.subMenus)
					RenderNode(child);

				ImGui::EndMenu();
			}

			return;
		}

		if (inst.isCheckbox)
		{
			if (ImGui::MenuItem(inst.displayName.c_str(), nullptr))
				inst.menu->OnExecute();
		}
		else
		{
			if (ImGui::MenuItem(inst.displayName.c_str()))
				inst.menu->OnExecute();
		}
	}

	void MenuRegistry::ClearRecursive(MenuItemInstance& inst)
	{
		Allocator::Delete(inst.menu);

		for (auto& newInst : inst.subMenus)
			ClearRecursive(newInst);
	}

	void MenuRegistry::SortRecursive(List<MenuItemInstance>& siblings)
	{
		siblings.Sort([](const MenuItemInstance& a, const MenuItemInstance& b)
			{
				if (a.order != b.order)
					return a.order < b.order;

				return a.displayName < b.displayName;
			});

		for (auto& child : siblings)
			SortRecursive(child.subMenus);
	}

	MenuItemInstance& MenuRegistry::FindOrCreateContainer(List<MenuItemInstance>& siblings, const std::string& name)
	{
		for (auto& child : siblings)
		{
			if (child.displayName == name)
				return child;
		}

		MenuItemInstance inst;
		inst.displayName = name;
		inst.order = 0x7FFFFFFF;
		inst.menu = nullptr;

		return siblings.EmplaceBack(std::move(inst));
	}
}