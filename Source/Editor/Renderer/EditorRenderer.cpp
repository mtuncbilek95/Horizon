#include "EditorRenderer.h"

#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/RHI/Texture/GfxTexture.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <backends/imgui_impl_dx12.h>

#include <fstream>
#include <vector>

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
		io.BackendFlags = ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
		io.DisplaySize = { 512.f, 512.f };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };

		m_device->InitializeImGui(m_graphicsQueue);

		m_commandLists.resize(3);
		for (u32 i = 0; i < 3; i++)
			m_commandLists[i] = m_device->CreateCommandList(GfxQueueType::Graphics);
	}

	EditorRenderer::~EditorRenderer()
	{
		for (GfxCommandList* cmd : m_commandLists)
			Allocator::Delete(cmd);

		m_device->ShutdownImGui();

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
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;

		// No need to flex. Just call this shit. Its Windows only editor.
		ImGui_ImplDX12_NewFrame();
		ImGui::NewFrame();

		return true;
	}

	b8 EditorRenderer::EndRender(GfxTexture* backbuffer, u32 imgIndex)
	{
		ImGui::Render();

		GfxCommandList* cmd = m_commandLists[imgIndex];
		cmd->Begin();
		cmd->SetupBindless();

		GfxTextureBarrier toTarget = { backbuffer, GfxResourceState::Present, GfxResourceState::RenderTarget };
		cmd->Barrier(&toTarget, 1);

		const GfxTextureDesc& bbDesc = backbuffer->GetDesc();

		GfxRenderBeginDesc pass = {};
		pass.addColorTarget(backbuffer, GfxLoadOp::Clear, { 0.1f, 0.1f, 0.1f, 1.0f })
			.setSize(bbDesc.width, bbDesc.height);
		cmd->BeginRendering(pass);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), (ID3D12GraphicsCommandList6*)cmd->GetAPIHandle());

		GfxTextureBarrier toPresent = { backbuffer, GfxResourceState::RenderTarget, GfxResourceState::Present };
		cmd->Barrier(&toPresent, 1);

		cmd->End();

		GfxCommandList* submitList[] = { cmd };
		m_graphicsQueue->Submit(submitList, 1);

		return true;
	}
}