#include "EditorRenderer.h"

#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <imgui.h>

namespace Horizon
{
	EditorRenderer::EditorRenderer(const EditorRendererDesc& desc) : m_device(desc.pDevice),
		m_graphicsQueue(desc.pQueue)
	{
		m_context = ImGui::CreateContext();
		ImGui::SetCurrentContext((ImGuiContext*)m_context);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags = ImGuiBackendFlags_None;
		io.DisplaySize = { 512.0f, 512.0f };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };
	}

	EditorRenderer::~EditorRenderer()
	{
		ImGui::DestroyContext((ImGuiContext*)m_context);
	}

	void EditorRenderer::OnMousePosition(i32 x, i32 y)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(x, y);
	}

	void EditorRenderer::OnMouseButtonDown(PAL::MouseButton button)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(ImGuiUtils::GetMouseButton(button), true);
	}

	void EditorRenderer::OnMouseButtonUp(PAL::MouseButton button)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(ImGuiUtils::GetMouseButton(button), false);
	}

	void EditorRenderer::OnMouseWheel(f32 delta)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(delta * (io.DeltaTime * 4), delta * (io.DeltaTime * 4));
	}

	void EditorRenderer::OnKeyboardDown(PAL::KeyCode key)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiUtils::GetKeyboardKey(key), true);

		if (key == PAL::KeyCode::LeftControl)
			io.AddKeyEvent(ImGuiKey_ReservedForModCtrl, true);
		if (key == PAL::KeyCode::LeftShift)
			io.AddKeyEvent(ImGuiKey_ReservedForModShift, true);
		if (key == PAL::KeyCode::LeftAlt)
			io.AddKeyEvent(ImGuiKey_ReservedForModAlt, true);
		if (key == PAL::KeyCode::LeftSuper)
			io.AddKeyEvent(ImGuiKey_ReservedForModSuper, true);
	}

	void EditorRenderer::OnKeyboardUp(PAL::KeyCode key)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiUtils::GetKeyboardKey(key), false);

		if (key == PAL::KeyCode::LeftControl)
			io.AddKeyEvent(ImGuiKey_ReservedForModCtrl, false);
		if (key == PAL::KeyCode::LeftShift)
			io.AddKeyEvent(ImGuiKey_ReservedForModShift, false);
		if (key == PAL::KeyCode::LeftAlt)
			io.AddKeyEvent(ImGuiKey_ReservedForModAlt, false);
		if (key == PAL::KeyCode::LeftSuper)
			io.AddKeyEvent(ImGuiKey_ReservedForModSuper, false);
	}

	void EditorRenderer::OnKeyboardChar(u32 value)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(value);
	}

	void EditorRenderer::OnResizeWindow(u32 width, u32 height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = { (f32)width,(f32)height };
	}

	b8 EditorRenderer::BeginRender(f32 dt)
	{
		return true;
	}

	b8 EditorRenderer::EndRender(/* Need some body like gpu fence, cpu fence, framebuffer etc. */)
	{
		return true;
	}
}