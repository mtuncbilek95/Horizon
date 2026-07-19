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

		LoadFonts();

		m_device->InitializeImGui(m_graphicsQueue);

		m_commandLists.resize(3);
		for (u32 i = 0; i < 3; i++)
			m_commandLists[i] = m_device->CreateCommandList(GfxQueueType::Graphics);

		DefaultStyle();
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

		pass.AddColorTarget(backbuffer, GfxLoadOp::Clear, { 0.1f, 0.1f, 0.1f, 1.0f })
			.SetSize(bbDesc.width, bbDesc.height);
		cmd->BeginRendering(pass);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), (ID3D12GraphicsCommandList6*)cmd->GetAPIHandle());

		cmd->EndRendering();

		GfxTextureBarrier toPresent = { backbuffer, GfxResourceState::RenderTarget, GfxResourceState::Present };

		cmd->Barrier(&toPresent, 1);

		cmd->End();

		GfxCommandList* submitList[] = { cmd };

		m_graphicsQueue->Submit(submitList, 1);

		return true;
	}

	void EditorRenderer::LoadFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		const std::string fontDir = std::string(HORIZON_RESOURCE_DIR) + "/Fonts/";

		constexpr f32 fontSize = 16.0f;

		const std::string bodyPath = fontDir + "SanFranciscoDisplay - Regular.OTF";

		ImFont* pBody = io.Fonts->AddFontFromFileTTF(bodyPath.c_str(), fontSize);
		if (!pBody)
		{
			Terminal::Error("EditorRenderer", "Failed to load UI Font: {}", bodyPath);
			io.Fonts->AddFontDefault();
			return;
		}

		const std::string iconPath = fontDir + "fa-solid-900.ttf";

		ImFontConfig iconCfg = {};
		iconCfg.MergeMode = true;
		iconCfg.PixelSnapH = true;
		iconCfg.ExtraSizeScale = 0.85f;
		iconCfg.GlyphMinAdvanceX = fontSize;
		iconCfg.GlyphOffset = ImVec2(0.0f, 0.f);

		static const ImWchar iconRange[] = { 0xe000, 0xf8ff, 0 };
		io.Fonts->AddFontFromFileTTF(iconPath.c_str(), fontSize, &iconCfg, iconRange);
	}

	void EditorRenderer::DefaultStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Common
		colors[ImGuiCol_Text] = ImGuiUtils::Hex("#CCCCCD");
		colors[ImGuiCol_TextDisabled] = ImGuiUtils::Hex("#6C6E76");
		colors[ImGuiCol_TextSelectedBg] = ImGuiUtils::Hex("#D4853A6E");
		colors[ImGuiCol_Border] = ImGuiUtils::Hex("#2A2C34");
		colors[ImGuiCol_BorderShadow] = ImGuiUtils::Hex("#00000000");
		style.ItemSpacing = { 5.0f, 5.0f };
		style.ItemInnerSpacing = { 12.0f, 8.0f };
		style.IndentSpacing = 25.0f;
		style.TouchExtraPadding = { 0.0f, 0.0f };
		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6f;
		style.ButtonTextAlign = { 0.5f, 0.5f };
		style.SelectableTextAlign = { 0.0f, 0.0f };

		// Window
		style.WindowPadding = { 16.0f, 16.0f };
		style.WindowMinSize = { 32.0f, 32.0f };
		style.WindowTitleAlign = { 0.0f, 0.5f };
		style.WindowRounding = 5.0f;
		style.WindowBorderSize = 1.0f;
		colors[ImGuiCol_WindowBg] = ImGuiUtils::Hex("#191B21");
		colors[ImGuiCol_TitleBg] = ImGuiUtils::Hex("#1C1E26");
		colors[ImGuiCol_TitleBgCollapsed] = ImGuiUtils::Hex("#191B21BF");
		colors[ImGuiCol_TitleBgActive] = ImGuiUtils::Hex("#15171D");

		// Child Window
		style.ChildRounding = 4.0f;
		style.ChildBorderSize = 1.0f;
		colors[ImGuiCol_ChildBg] = ImGuiUtils::Hex("#00000000");

		// Main Menu Bar
		colors[ImGuiCol_MenuBarBg] = ImGuiUtils::Hex("#1C1E26");

		// Pop-up & Context Menu
		style.PopupRounding = 4.0f;
		style.PopupBorderSize = 1.0f;
		colors[ImGuiCol_PopupBg] = ImGuiUtils::Hex("#15171D");

		// Button
		colors[ImGuiCol_Button] = ImGuiUtils::Hex("#1C1E26");
		colors[ImGuiCol_ButtonHovered] = ImGuiUtils::Hex("#2E3038");
		colors[ImGuiCol_ButtonActive] = ImGuiUtils::Hex("#D4853A");

		// Frame
		style.FramePadding = { 4.0f, 4.0f };
		style.FrameRounding = 4.0f;
		style.FrameBorderSize = 1.0f;
		colors[ImGuiCol_FrameBg] = ImGuiUtils::Hex("#111318");
		colors[ImGuiCol_FrameBgHovered] = ImGuiUtils::Hex("#2E3038");
		colors[ImGuiCol_FrameBgActive] = ImGuiUtils::Hex("#3A3C44");

		// Header
		colors[ImGuiCol_Header] = ImGuiUtils::Hex("#1C1E26");
		colors[ImGuiCol_HeaderHovered] = ImGuiUtils::Hex("#2E3038");
		colors[ImGuiCol_HeaderActive] = ImGuiUtils::Hex("#15171D");

		// Scrollbar
		style.ScrollbarSize = 11.0f;
		style.ScrollbarRounding = 9.0f;
		colors[ImGuiCol_ScrollbarBg] = ImGuiUtils::Hex("#191B21");
		colors[ImGuiCol_ScrollbarGrab] = ImGuiUtils::Hex("#3A3C4480");
		colors[ImGuiCol_ScrollbarGrabHovered] = ImGuiUtils::Hex("#4A4C54");
		colors[ImGuiCol_ScrollbarGrabActive] = ImGuiUtils::Hex("#6C6E76");

		// Slider & Grab
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 3.0f;
		colors[ImGuiCol_SliderGrab] = ImGuiUtils::Hex("#D4853A");
		colors[ImGuiCol_SliderGrabActive] = ImGuiUtils::Hex("#E08A3E");

		// Checkbox & Radio
		colors[ImGuiCol_CheckMark] = ImGuiUtils::Hex("#D4853A");

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImGuiUtils::Hex("#00000000");
		colors[ImGuiCol_ResizeGripHovered] = ImGuiUtils::Hex("#2E3038");
		colors[ImGuiCol_ResizeGripActive] = ImGuiUtils::Hex("#D4853A");

		// Tab
		style.TabRounding = 4.0f;
		style.TabBorderSize = 1.0f;
		style.TabBarBorderSize = 1.0f;
		colors[ImGuiCol_Tab] = ImGuiUtils::Hex("#15171D");
		colors[ImGuiCol_TabHovered] = ImGuiUtils::Hex("#2E3038");
		colors[ImGuiCol_TabSelected] = ImGuiUtils::Hex("#191B21");
		colors[ImGuiCol_TabDimmed] = ImGuiUtils::Hex("#111318");
		colors[ImGuiCol_TabDimmedSelected] = ImGuiUtils::Hex("#1C1E26");

		// Separator
		colors[ImGuiCol_Separator] = ImGuiUtils::Hex("#2A2C34");
		colors[ImGuiCol_SeparatorHovered] = ImGuiUtils::Hex("#D4853A");
		colors[ImGuiCol_SeparatorActive] = ImGuiUtils::Hex("#E08A3E");

		// Table
		style.CellPadding = { 4.0f, 2.0f };
		colors[ImGuiCol_TableHeaderBg] = ImGuiUtils::Hex("#1C1E26");
		colors[ImGuiCol_TableBorderStrong] = ImGuiUtils::Hex("#2A2C34");
		colors[ImGuiCol_TableBorderLight] = ImGuiUtils::Hex("#22242C");
		colors[ImGuiCol_TableRowBg] = ImGuiUtils::Hex("#00000000");
		colors[ImGuiCol_TableRowBgAlt] = ImGuiUtils::Hex("#FFFFFF06");

		// Docking
		colors[ImGuiCol_DockingPreview] = ImGuiUtils::Hex("#D4853AB2");
		colors[ImGuiCol_DockingEmptyBg] = ImGuiUtils::Hex("#191B21");

		// Plot / Graph
		colors[ImGuiCol_PlotLines] = ImGuiUtils::Hex("#CCCCCD");
		colors[ImGuiCol_PlotLinesHovered] = ImGuiUtils::Hex("#E08A3E");
		colors[ImGuiCol_PlotHistogram] = ImGuiUtils::Hex("#D4853A");
		colors[ImGuiCol_PlotHistogramHovered] = ImGuiUtils::Hex("#E08A3E");

		// Drag & Drop
		colors[ImGuiCol_DragDropTarget] = ImGuiUtils::Hex("#D4853AE5");

		// Navigation
		colors[ImGuiCol_NavCursor] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		colors[ImGuiCol_NavWindowingHighlight] = ImGuiUtils::Hex("#FFFFFFB2");
		colors[ImGuiCol_NavWindowingDimBg] = ImGuiUtils::Hex("#CCCCCC33");
		colors[ImGuiCol_ModalWindowDimBg] = ImGuiUtils::Hex("#CCCCCC59");

		// Shape Rendering
		style.CircleTessellationMaxError = 0.3f;
		style.CurveTessellationTol = 1.25f;
		style.WindowMenuButtonPosition = ImGuiDir_None;
	}

}