#include "ViewRegistry.h"

#include <Editor/Attributes/EditorViewAttribute.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon::Editor
{
	ViewRegistry::ViewRegistry()
	{
	}

	ViewRegistry::~ViewRegistry()
	{
		for (auto* view : m_createdViews)
			Memory::Allocator::Delete(view);

		m_createdViews.Clear();
		m_registeredViews.Clear();
	}

	void ViewRegistry::BootstrapViews(const EditorContext& ctx)
	{
		m_context = ctx;

		if (!ctx.pEngine)
		{
			Terminal::Fatal(StringOps::GetName(this), "Somehow engine is not there!");
			return;
		}

		for (auto* view : m_createdViews)
			Memory::Allocator::Delete(view);

		m_createdViews.Clear();
		m_registeredViews.Clear();

		auto* pReflect = ctx.pEngine->GetReflectionSystem();

		List<Reflect::Type*> types = pReflect->GetTypeByAttribute(Reflect::TypeOf<EditorViewAttribute>());

		for (auto* type : types)
		{
			if (type->GetBaseId() != Reflect::TypeOf<ViewObject>())
			{
				Terminal::Error(StringOps::GetName(this), "{} has not inherited from ViewObject. Please inherit then restart engine!", type->GetName());
				continue;
			}

			auto* pAttr = type->GetCustomAttribute<EditorViewAttribute>();

			m_registeredViews.PushBack(ViewDescriptor
				{
					.displayName = pAttr->GetDisplayName(),
					.multiInstance = pAttr->GetMultiInstance(),
					.openOnStart = pAttr->GetOpenOnStart(),
					.dockZone = pAttr->GetDock(),
					.pCoreType = type
				});
		}

		for (const auto& view : m_registeredViews)
		{
			if (view.openOnStart)
			{
				auto* viewObj = (ViewObject*)view.pCoreType->Create();
				viewObj->m_context = &m_context;
				viewObj->m_displayName = view.displayName;
				viewObj->m_holder = this;

				viewObj->OnInvoke();

				m_createdViews.PushBack(viewObj);
			}
		}
	}

	void ViewRegistry::RenderGUI()
	{
		ImGuiViewport* pViewport = ImGui::GetMainViewport();

		ImGuiDockNodeFlags dockFlags =
			ImGuiDockNodeFlags_PassthruCentralNode |
			ImGuiDockNodeFlags_NoWindowMenuButton;

		ImGuiID dockId = ImGui::DockSpaceOverViewport(0, pViewport, dockFlags);

		if (!m_layoutDirty)
		{
			BuildDefaultLayout(dockId);
			m_layoutDirty = true;
		}

		for (auto* view : m_createdViews)
		{
			if (view->IsFullBleed())
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			}

			ImGui::Begin(view->m_displayName.c_str());

			if (view->IsFullBleed())
				ImGui::PopStyleVar(2);

			view->OnRender();
			ImGui::End();
		}
	}

	void ViewRegistry::BuildDefaultLayout(u32 rootId)
	{
		ImGui::DockBuilderRemoveNode(rootId);
		ImGui::DockBuilderAddNode(rootId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(rootId, ImGui::GetMainViewport()->Size);

		ImGuiID center = rootId;
		ImGuiID left, right, bottom;
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, &bottom, &center);
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f, &left, &center);
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, &right, &center);

		for (const auto& view : m_registeredViews)
		{
			ImGuiID target = center;
			switch (view.dockZone)
			{
			case DockZone::Left:
				target = left;
				break;
			case DockZone::Right:
				target = right;
				break;
			case DockZone::Bottom:
				target = bottom;
				break;
			default: 
				break;
			}

			ImGui::DockBuilderDockWindow(view.displayName.c_str(), target);
		}

		ImGui::DockBuilderFinish(rootId);
	}

	ViewObject* ViewRegistry::GetViewObject(Reflect::TypeHandle handl)
	{
		for (auto* view : m_createdViews)
		{
			if (view->GetTypeId() == handl)
				return view;
		}

		auto* pReflect = m_context.pEngine->GetReflectionSystem();
		std::string_view handlName = pReflect->GetType(handl)->GetName();
		Terminal::Error(StringOps::GetName(this), "Could not find {}", handlName);
		return nullptr;
	}
}