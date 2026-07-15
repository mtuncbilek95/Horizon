#include "WidgetRegistry.h"

#include <Editor/Widget/IWidget.h>
#include <Editor/Widget/WidgetAttribute.h>
#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon
{
	WidgetRegistry::WidgetRegistry(Engine* pEngine) : m_engine(pEngine)
	{
	}

	WidgetRegistry::~WidgetRegistry()
	{
		for (OpenWidget& inst : m_open)
			Allocator::Delete(inst.widget);
	}

	void WidgetRegistry::Invalidate()
	{
		for (OpenWidget& inst : m_open)
			Allocator::Delete(inst.widget);

		m_open.clear();
		m_types.clear();

		ModuleContext* modCtx = m_engine->GetModuleContext();
		auto manifests = modCtx->GetManifestsByBase(TypeIdOf<IWidget>());

		for (TypeManifest* manifest : manifests)
		{
			WidgetTypeAttribute* attr = manifest->GetCustomAttribute<WidgetTypeAttribute>();
			if (!attr)
			{
				Terminal::Warn("WidgetRegistry", "IWidget manifest without WidgetTypeAttribute, skipping");
				continue;
			}

			std::string title = std::string(attr->GetIcon()) + "  " + std::string(attr->GetDisplayName());
			m_types.push_back({ manifest, std::move(title), attr->GetDockLayout(), attr->IsAlwaysOpenFirst() });
		}

		for (const WidgetType& type : m_types)
		{
			if (type.alwaysOpenFirst)
				Open(type);
		}

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

		for (usize i = 0; i < m_open.size();)
		{
			OpenWidget& inst = m_open[i];

			if (ImGui::Begin(inst.title.c_str(), &inst.open))
				inst.widget->OnDraw();

			ImGui::End();

			if (!inst.open)
			{
				Allocator::Delete(inst.widget);
				m_open.erase(m_open.begin() + i);
				continue;
			}

			++i;
		}
	}

	void WidgetRegistry::Open(const WidgetType& type)
	{
		for (const OpenWidget& inst : m_open)
		{
			if (inst.manifest == type.widgetManifest)
			{
				ImGui::SetWindowFocus(inst.title.c_str());
				return;
			}
		}

		IWidget* widget = static_cast<IWidget*>(type.widgetManifest->Create());
		if (!widget)
		{
			Terminal::Warn("WidgetRegistry", "manifest->Create() returned null");
			return;
		}

		widget->SetEngine(m_engine);
		widget->OnInvoke();

		m_open.push_back({ widget, type.widgetManifest, type.widgetDisplayName, true });
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

		for (const WidgetType& type : m_types)
		{
			ImGuiID target = center;
			switch (type.dock)
			{
			case DockLayout::Left:
				target = left;
				break;
			case DockLayout::Right:
				target = right;
				break;
			case DockLayout::Bottom:
				target = bottom;
				break;
			case DockLayout::Center:
				target = center;
				break;
			}

			ImGui::DockBuilderDockWindow(type.widgetDisplayName.c_str(), target);
		}

		ImGui::DockBuilderFinish(rootId);
	}
}