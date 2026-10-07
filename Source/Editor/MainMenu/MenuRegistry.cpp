#include "MenuRegistry.h"

#include <Editor/MainMenu/MainMenuItemAttribute.h>
#include <Editor/MainMenu/MenuItemAttribute.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/PAL/Window/Window.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon::Editor
{
	MenuRegistry::~MenuRegistry()
	{
		for (auto& inst : m_menus)
			ClearRecursive(inst);
		m_menus.Clear();
	}

	void MenuRegistry::BootstrapMenus(const EditorContext& ctx)
	{
		m_context = ctx;
		m_window = ctx.pWindow;

		for (auto& inst : m_menus)
			ClearRecursive(inst);
		m_menus.Clear();

		auto* pReflect = ctx.pEngine->GetReflectionSystem();

		for (auto* pType : pReflect->GetTypeByAttribute(Reflect::TypeOf<MainMenuItemAttribute>()))
			AddRootType(pType);

		for (auto* pType : pReflect->GetTypeByAttribute(Reflect::TypeOf<MenuItemAttribute>()))
			AddLeafType(pType);

		SortRecursive(m_menus);
	}

	void MenuRegistry::RenderGUI()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar;

		const f32 padY = (kHeight - ImGui::GetFontSize()) * 0.5f;

		PAL::WindowChrome chrome = {};
		chrome.captionHeight = u32(kHeight);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, padY));

		const b8 barOpen = ImGui::BeginViewportSideBar("##HorizonTitleBar", pViewport, ImGuiDir_Up, kHeight, flags);
		const b8 menuOpen = barOpen && ImGui::BeginMenuBar();

		ImGui::PopStyleVar(3);

		if (menuOpen)
		{
			const ImVec2 origin = pViewport->Pos;

			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1], 0.f);
			ImGui::TextUnformatted(" " ICON_FA_METEOR " ");
			ImGui::PopFont();

			for (auto& root : m_menus)
				RenderNode(root);

			const f32 menusEnd = ImGui::GetCursorScreenPos().x - origin.x;

			PAL::ChromeRect menuArea = {};
			menuArea.x = 0;
			menuArea.y = 0;
			menuArea.w = i32(menusEnd);
			menuArea.h = i32(kHeight);
			chrome.clientAreas.PushBack(menuArea);

			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1], 0.f); // Title Bold Start
			const f32 buttonsStart = pViewport->Size.x - kButtonWidth * 3.0f;
			const std::string& title = m_window->GetName();
			const f32 titleWidth = ImGui::CalcTextSize(title.c_str()).x;
			const f32 titleX = (pViewport->Size.x - titleWidth) * 0.5f;

			if (titleX > menusEnd + 16.0f && titleX + titleWidth < buttonsStart - 16.0f)
			{
				ImGui::SetCursorPosX(titleX);
				ImGui::TextDisabled("%s", title.c_str());
			}
			ImGui::PopFont(); // Title Bold End

			ImGui::SetCursorPosX(buttonsStart);

			const c8* pMaximizeIcon = m_window->GetMaximized() ? ICON_FA_WINDOW_RESTORE : ICON_FA_WINDOW_MAXIMIZE;

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			chrome.minimizeButton = RenderChromeButton(ICON_FA_WINDOW_MINIMIZE "##Minimize", false);
			chrome.maximizeButton = RenderChromeButton(pMaximizeIcon, false);
			chrome.closeButton = RenderChromeButton(ICON_FA_XMARK "##Close", true);
			ImGui::PopStyleVar();

			ImGui::EndMenuBar();
		}

		ImGui::End();

		if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		{
			PAL::ChromeRect fullStrip = {};
			fullStrip.x = 0;
			fullStrip.y = 0;
			fullStrip.w = i32(pViewport->Size.x);
			fullStrip.h = i32(kHeight);
			chrome.clientAreas.PushBack(fullStrip);
		}

		m_window->SetChrome(chrome);
	}

	void MenuRegistry::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
		{
			if (type.GetCustomAttribute<MainMenuItemAttribute>())
				AddRootType(&type);
		}

		for (const Reflect::Type& type : library.types)
		{
			if (type.GetCustomAttribute<MenuItemAttribute>())
				AddLeafType(&type);
		}

		SortRecursive(m_menus);
	}

	void MenuRegistry::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			RemoveType(m_menus, &type);
	}

	void MenuRegistry::AddRootType(const Reflect::Type* pType)
	{
		auto* pAttr = pType->GetCustomAttribute<MainMenuItemAttribute>();

		if (!pAttr)
			return;

		MenuItemInstance& inst = FindOrCreateContainer(m_menus, pAttr->GetPath());
		inst.order = pAttr->GetOrder();
		inst.pType = pType;
	}

	void MenuRegistry::AddLeafType(const Reflect::Type* pType)
	{
		auto* pAttr = pType->GetCustomAttribute<MenuItemAttribute>();

		if (!pAttr)
			return;

		const std::string& path = pAttr->GetPath();
		i32 order = pAttr->GetOrder();

		List<MenuItemInstance>* pLevel = &m_menus;
		usize start = 0;
		b8 rootSegment = true;

		while (true)
		{
			const usize slash = path.find('/', start);
			const b8 isLeaf = (slash == std::string::npos);
			std::string segment = path.substr(start, isLeaf ? std::string::npos : slash - start);

			if (isLeaf)
			{
				auto* pMenuObj = static_cast<MenuItem*>(pType->Create());

				if (!pMenuObj)
				{
					Terminal::Warn(StringOps::GetName(this), "{} could not be created, forgetting a virtual implementation causes this", pType->GetName());
					return;
				}

				pMenuObj->m_context = &m_context;

				MenuItemInstance leaf;
				leaf.displayName = std::move(segment);
				leaf.isCheckbox = pAttr->GetIsCheckbox();
				leaf.order = order;
				leaf.pMenu = pMenuObj;
				leaf.pType = pType;

				pLevel->PushBack(std::move(leaf));
				return;
			}

			MenuItemInstance& container = FindOrCreateContainer(*pLevel, segment);

			if (!rootSegment && order < container.order)
				container.order = order;

			pLevel = &container.subMenus;
			start = slash + 1;
			rootSegment = false;
		}
	}

	b8 MenuRegistry::RemoveType(List<MenuItemInstance>& siblings, const Reflect::Type* pType)
	{
		b8 removed = false;

		for (usize i = siblings.GetCount(); i > 0; --i)
		{
			MenuItemInstance& inst = siblings[i - 1];

			if (inst.pType == pType)
			{
				ClearRecursive(inst);
				siblings.RemoveAt(i - 1);
				removed = true;
				continue;
			}

			if (!RemoveType(inst.subMenus, pType))
				continue;

			removed = true;

			if (inst.pType == nullptr && inst.pMenu == nullptr && inst.subMenus.IsEmpty())
				siblings.RemoveAt(i - 1);
		}

		return removed;
	}

	void MenuRegistry::RenderNode(MenuItemInstance& inst)
	{
		if (inst.pMenu == nullptr)
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
				inst.pMenu->OnExecute();
		}
		else
		{
			if (ImGui::MenuItem(inst.displayName.c_str()))
				inst.pMenu->OnExecute();
		}
	}

	void MenuRegistry::ClearRecursive(MenuItemInstance& inst)
	{
		Memory::Allocator::Delete(inst.pMenu);

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
		inst.pMenu = nullptr;

		return siblings.EmplaceBack(std::move(inst));
	}

	PAL::ChromeRect MenuRegistry::RenderChromeButton(const c8* pLabel, b8 danger)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImGuiUtils::Hex("#00000000"));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, danger ? ImGuiUtils::Hex("#C42B1C") : ImGuiUtils::Hex("#3C3C3C"));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, danger ? ImGuiUtils::Hex("#E81123") : ImGuiUtils::Hex("#4A4A4A"));

		ImGui::Button(pLabel, ImVec2(kButtonWidth, kHeight));

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);

		const ImVec2 origin = ImGui::GetMainViewport()->Pos;
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();

		PAL::ChromeRect rect = {};
		rect.x = i32(min.x - origin.x);
		rect.y = i32(min.y - origin.y);
		rect.w = i32(max.x - min.x);
		rect.h = i32(max.y - min.y);

		return rect;
	}
}