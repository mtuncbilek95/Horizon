#include "WidgetRegistry.h"

#include <Editor/Widget/IWidget.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon
{
	WidgetRegistry::WidgetRegistry(Engine* pEngine) : m_engine(pEngine)
	{
	}

	WidgetRegistry::~WidgetRegistry()
	{
		for (auto& inst : m_widgets)
			Allocator::Delete(inst.widget);
	}

	void WidgetRegistry::Invalidate()
	{
		for (auto& inst : m_widgets)
			Allocator::Delete(inst.widget);

		m_widgets.clear();

		WidgetFactory::Get().ForEach([this](const WidgetTypeInfo& info)
			{
				if (info.alwaysOpenFirst)
					Open(info);
			});

		m_layout = false;
	}

	void WidgetRegistry::Render()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();

		ImGuiDockNodeFlags dockFlags =
			ImGuiDockNodeFlags_PassthruCentralNode |
			ImGuiDockNodeFlags_NoWindowMenuButton;
		ImGuiID dockId = ImGui::DockSpaceOverViewport(0, pViewport, dockFlags);

		if (!m_layout)
		{
			BuildDefaultLayout(dockId);
			m_layout = true;
		}

		for (usize i = 0; i < m_widgets.size();)
		{
			WidgetInstance& inst = m_widgets[i];

			if (ImGui::Begin(inst.title.data(), &inst.isOpen))
				inst.widget->OnDraw();

			ImGui::End();

			if (!inst.isOpen)
			{
				Allocator::Delete(inst.widget);
				m_widgets.erase(m_widgets.begin() + i);
				continue;
			}

			++i;
		}
	}

	void WidgetRegistry::Open(const WidgetTypeInfo& info)
	{
		for (auto& inst : m_widgets)
		{
			if (inst.type == info.type)
			{
				ImGui::SetWindowFocus(inst.title.c_str());
				return;
			}
		}

		IWidget* pWidget = info.CreateWidget(m_engine);
		if (!pWidget)
		{
			Terminal::Warn("WidgetRegistry", "Factory returned null for a widget type");
			return;
		}

		pWidget->OnInvoke();

		std::string title;
		if (!info.icon.empty())
		{
			title += info.icon;
			title += " ";
		}

		title += info.name;
		title += "###";
		title += info.name;

		m_widgets.emplace_back(pWidget, std::move(title), info.dock, true, info.type);
		m_lookup[info.type] = m_widgets.size() - 1;
	}

	void WidgetRegistry::Open(const std::type_index& index)
	{
		auto& value = WidgetFactory::Get().GetByInfo(index);
		Open(value);
	}

	void WidgetRegistry::Close(const std::type_index& index)
	{
		for (usize i = 0; i < m_widgets.size(); ++i)
		{
			if (m_widgets[i].type != index)
				continue;

			Allocator::Delete(m_widgets[i].widget);
			m_widgets.erase(m_widgets.begin() + i);
			m_lookup.erase(index);
			return;
		}
	}

	b8 WidgetRegistry::IsOpened(const std::type_index& index)
	{
		auto it = m_lookup.find(index);
		if (it == m_lookup.end())
			return false;

		auto& inst = m_widgets[it->second];
		return inst.isOpen;
	}

	void WidgetRegistry::BuildDefaultLayout(u32 rootId)
	{
		ImGui::DockBuilderRemoveNode(rootId);
		ImGui::DockBuilderAddNode(rootId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(rootId, ImGui::GetMainViewport()->Size);

		ImGuiID center = rootId;
		ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f, nullptr, &center);
		ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
		ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);

		for (auto& inst : m_widgets)
		{
			ImGuiID target = center;
			switch (inst.dock)
			{
			case WidgetDock::Left:
				target = left;   
				break;
			case WidgetDock::Right:
				target = right;  
				break;
			case WidgetDock::Bottom:
				target = bottom; 
				break;
			case WidgetDock::Center:
				target = center; 
				break;
			}

			ImGui::DockBuilderDockWindow(inst.title.data(), target);
		}

		ImGui::DockBuilderFinish(rootId);
	}
}