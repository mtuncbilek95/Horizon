#include "ViewRegistry.h"

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewCommandBinding.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <format>

namespace Horizon::Editor
{
	ViewRegistry::ViewRegistry()
	{
	}

	ViewRegistry::~ViewRegistry()
	{
		for (auto* pView : m_createdViews)
			Memory::Allocator::Delete(pView);

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

		for (auto* pView : m_createdViews)
			Memory::Allocator::Delete(pView);

		m_createdViews.Clear();
		m_registeredViews.Clear();

		auto* pReflect = ctx.pEngine->GetReflectionSystem();

		for (auto* pType : pReflect->GetTypeByAttribute(Reflect::TypeOf<EditorViewAttribute>()))
			AddType(pType);
	}

	void ViewRegistry::RenderGUI(const Engine::EngineFrame& context)
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

		for (const ViewCommandBinding& binding : ViewCommandBindings)
		{
			if (!ImGui::Shortcut(binding.broadcastChord, ImGuiInputFlags_RouteGlobal))
				continue;

			for (auto* pView : m_createdViews)
				pView->OnCommand(binding.command);
		}

		List<ViewObject*> closing;

		for (auto* pView : m_createdViews)
		{
			const b8 closable = HasFlag(pView->m_flags, EditorViewFlags::Mutable);

			if (pView->IsFullBleed())
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			}

			if (pView->m_focusRequested)
			{
				ImGui::SetNextWindowFocus();
				pView->m_focusRequested = false;
			}

			const b8 visible = ImGui::Begin(pView->m_windowTitle.c_str(), closable ? &pView->m_open : nullptr);

			if (pView->IsFullBleed())
				ImGui::PopStyleVar(2);

			if (visible)
			{
				for (const ViewCommandBinding& binding : ViewCommandBindings)
				{
					if (ImGui::Shortcut(binding.focusedChord))
						pView->OnCommand(binding.command);
				}

				pView->OnRender(context);
			}

			ImGui::End();

			if (closable && !pView->m_open)
				closing.PushBack(pView);
		}

		for (auto* pView : closing)
			CloseView(pView);
	}

	void ViewRegistry::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
		{
			if (type.GetCustomAttribute<EditorViewAttribute>())
				AddType(&type);
		}

		for (auto* pView : m_createdViews)
			pView->OnLibraryRegistered(library);
	}

	void ViewRegistry::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		for (auto* pView : m_createdViews)
			pView->OnLibraryUnregistered(library);

		for (const Reflect::Type& type : library.types)
		{
			for (usize i = m_createdViews.GetCount(); i > 0; --i)
			{
				if (m_createdViews[i - 1]->GetTypeId() == type.GetTypeId())
					CloseView(m_createdViews[i - 1]);
			}

			for (usize i = m_registeredViews.GetCount(); i > 0; --i)
			{
				if (m_registeredViews[i - 1].pCoreType == &type)
					m_registeredViews.RemoveAt(i - 1);
			}
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
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.15f, &left, &center);
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.15f, &right, &center);

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

	b8 ViewRegistry::HasViewObject(Reflect::TypeHandle handl)
	{
		for (auto* pView : m_createdViews)
		{
			if (pView->GetTypeId() == handl)
				return true;
		}

		return false;
	}

	ViewObject* ViewRegistry::GetViewObject(Reflect::TypeHandle handl)
	{
		for (auto* pView : m_createdViews)
		{
			if (pView->GetTypeId() == handl)
				return pView;
		}

		auto* pReflect = m_context.pEngine->GetReflectionSystem();
		std::string_view handlName = pReflect->GetType(handl)->GetName();
		Terminal::Error(StringOps::GetName(this), "Could not find {}", handlName);
		return nullptr;
	}

	ViewObject* ViewRegistry::OpenView(Reflect::TypeHandle handl)
	{
		const ViewDescriptor* pDescriptor = FindDescriptor(handl);

		if (!pDescriptor)
		{
			Terminal::Error(StringOps::GetName(this), "OpenView could not find a registered view for the given type");
			return nullptr;
		}

		if (!HasFlag(pDescriptor->flags, EditorViewFlags::MultiInstance))
		{
			for (auto* pView : m_createdViews)
			{
				if (pView->GetTypeId() == handl)
				{
					pView->m_focusRequested = true;
					return pView;
				}
			}
		}

		ViewObject* pView = CreateView(*pDescriptor);
		pView->m_focusRequested = true;
		return pView;
	}

	void ViewRegistry::CloseView(ViewObject* pView)
	{
		if (!pView)
			return;

		if (!m_createdViews.Remove(pView))
		{
			Terminal::Warn(StringOps::GetName(this), "{} is not an open view, nothing to close", pView->GetDisplayName());
			return;
		}

		Memory::Allocator::Delete(pView);
	}

	b8 ViewRegistry::AddType(const Reflect::Type* pType)
	{
		if (pType->GetBaseId() != Reflect::TypeOf<ViewObject>())
		{
			Terminal::Error(StringOps::GetName(this), "{} has not inherited from ViewObject. Please inherit then restart engine!", pType->GetName());
			return false;
		}

		auto* pAttr = pType->GetCustomAttribute<EditorViewAttribute>();

		if (!pAttr)
			return false;

		const EditorViewFlags flags = pAttr->GetFlags();

		if (HasFlag(flags, EditorViewFlags::Mutable) && HasFlag(flags, EditorViewFlags::OpenOnStart))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be both Mutable & OpenOnStart. Wont have this view in the system", pType->GetName());
			return false;
		}

		if (!HasFlag(flags, EditorViewFlags::Mutable) && !HasFlag(flags, EditorViewFlags::OpenOnStart))
		{
			Terminal::Error(StringOps::GetName(this), "{} is neither Mutable nor OpenOnStart, it could never be closed once opened. Wont have this view in the system", pType->GetName());
			return false;
		}

		m_registeredViews.PushBack(ViewDescriptor
			{
				.displayName = pAttr->GetDisplayName(),
				.flags = flags,
				.dockZone = pAttr->GetDock(),
				.pCoreType = pType
			});

		if (HasFlag(flags, EditorViewFlags::OpenOnStart))
			CreateView(m_registeredViews.Back());

		return true;
	}

	const ViewDescriptor* ViewRegistry::FindDescriptor(Reflect::TypeHandle handl) const
	{
		for (const auto& view : m_registeredViews)
		{
			if (view.pCoreType->GetTypeId() == handl)
				return &view;
		}

		return nullptr;
	}

	ViewObject* ViewRegistry::CreateView(const ViewDescriptor& descriptor)
	{
		auto* pView = (ViewObject*)descriptor.pCoreType->Create();
		pView->m_context = &m_context;
		pView->m_holder = this;
		pView->m_displayName = descriptor.displayName;
		pView->m_flags = descriptor.flags;
		pView->m_open = true;

		if (HasFlag(descriptor.flags, EditorViewFlags::MultiInstance))
			pView->m_windowTitle = std::format("{}###{}_{}", descriptor.displayName, descriptor.pCoreType->GetName(), m_instanceCounter++);
		else
			pView->m_windowTitle = descriptor.displayName;

		pView->OnInvoke();
		m_createdViews.PushBack(pView);
		return pView;
	}
}