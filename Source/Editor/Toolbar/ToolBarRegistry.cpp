#include "ToolBarRegistry.h"

#include <Editor/Attributes/ToolBarItemAttribute.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon::Editor
{
	ToolBarRegistry::~ToolBarRegistry()
	{
		Clear();
	}

	void ToolBarRegistry::BootstrapItems(const EditorContext& ctx)
	{
		Clear();
		m_context = ctx;

		auto* pReflect = ctx.pEngine->GetReflectionSystem();
		List<Reflect::Type*> types = pReflect->GetTypeByAttribute(Reflect::TypeOf<ToolBarItemAttribute>());

		for (auto* pType : types)
		{
			if (pType->GetBaseId() != Reflect::TypeOf<ToolBarItem>())
			{
				Terminal::Error(StringOps::GetName(this), "{} has not inherited from ToolBarItem.", pType->GetName());
				continue;
			}

			auto* pAttr = pType->GetCustomAttribute<ToolBarItemAttribute>();
			auto* pItem = static_cast<ToolBarItem*>(pType->Create());
			if (!pItem)
			{
				Terminal::Warn(StringOps::GetName(this), "Previous error may cause due to forgetting virtual function implementations");
				continue;
			}

			pItem->m_context = &m_context;
			m_sections[usize(pAttr->GetSection())].EmplaceBack(pItem, pAttr->GetOrder());
		}

		for (auto& section : m_sections)
		{
			section.Sort([](const ItemEntry& a, const ItemEntry& b)
				{
					return a.order < b.order;
				});
		}
	}

	void ToolBarRegistry::RenderGUI()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse;

		const f32 padY = (kHeight - ImGui::GetFrameHeight()) * 0.5f;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, padY));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		const b8 barOpen = ImGui::BeginViewportSideBar("##HorizonToolBar", pViewport, ImGuiDir_Up, kHeight, flags);

		ImGui::PopStyleVar(2);

		if (barOpen)
		{
			const f32 width = ImGui::GetContentRegionAvail().x;
			const f32 startX = ImGui::GetCursorPosX();

			RenderSection(ToolBarSection::Left, startX);
			RenderSection(ToolBarSection::Center, startX + (width - m_sectionWidths[usize(ToolBarSection::Center)]) * 0.5f);
			RenderSection(ToolBarSection::Right, startX + width - m_sectionWidths[usize(ToolBarSection::Right)]);
		}

		ImGui::End();
	}

	void ToolBarRegistry::RenderSection(ToolBarSection section, f32 cursorX)
	{
		List<ItemEntry>& items = m_sections[usize(section)];
		if (items.IsEmpty())
			return;

		ImGui::SameLine();
		ImGui::SetCursorPosX(cursorX);

		ImGui::BeginGroup();
		for (usize i = 0; i < items.GetCount(); i++)
		{
			if (i > 0)
				ImGui::SameLine();

			ImGui::PushID(items[i].pItem);
			items[i].pItem->OnRender();
			ImGui::PopID();
		}
		ImGui::EndGroup();

		m_sectionWidths[usize(section)] = ImGui::GetItemRectSize().x;
	}

	void ToolBarRegistry::Clear()
	{
		for (auto& section : m_sections)
		{
			for (auto& entry : section)
				Memory::Allocator::Delete(entry.pItem);

			section.Clear();
		}
	}
}