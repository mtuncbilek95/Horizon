#include "EditorRenderer.h"

#include <Editor/Font/IconsFontAudio.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Font/IconsKenney.h>
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

namespace Horizon::Editor
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

		m_commandLists.Resize(3);
		for (u32 i = 0; i < 3; i++)
			m_commandLists[i] = m_device->CreateCommandList(GfxQueueType::Graphics);

		DefaultStyle();
	}

	EditorRenderer::~EditorRenderer()
	{
		for (GfxCommandList* cmd : m_commandLists)
			Memory::Allocator::Delete(cmd);

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

		m_device->NewFrameImGui();
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
		if (!io.Fonts->AddFontFromFileTTF(bodyPath.c_str(), fontSize))
		{
			Terminal::Error("EditorRenderer", "Failed to load UI Font: {}", bodyPath);
			io.Fonts->AddFontDefault();
		}

		ImFontConfig iconCfg = {};
		iconCfg.MergeMode = true;
		iconCfg.PixelSnapH = true;
		iconCfg.ExtraSizeScale = 0.85f;
		iconCfg.GlyphMinAdvanceX = fontSize;
		iconCfg.GlyphOffset = ImVec2(0.0f, 0.0f);

		const auto mergeIconFont = [&](const char* file, const ImWchar* range,
			const ImWchar* exclude = nullptr)
			{
				iconCfg.GlyphExcludeRanges = exclude;
				const std::string path = fontDir + file;
				if (!io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize, &iconCfg, range))
					Terminal::Error("EditorRenderer", "Failed to load icon font: {}", path);
			};

		static const ImWchar faRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		static const ImWchar kiRange[] = { ICON_MIN_KI, ICON_MAX_KI, 0 };

		mergeIconFont("fa-solid-900.ttf", faRange);
		mergeIconFont("kenney-icon-font.ttf", kiRange);
	}

	void EditorRenderer::DefaultStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Common
		colors[ImGuiCol_Text] = ImGuiUtils::Hex("#FFFFFF");
		colors[ImGuiCol_TextDisabled] = ImGuiUtils::Hex("#808080");
		colors[ImGuiCol_TextSelectedBg] = ImGuiUtils::Hex("#0E79D0");
		colors[ImGuiCol_Border] = ImGuiUtils::Hex("#383838");
		colors[ImGuiCol_BorderShadow] = ImGuiUtils::Hex("#00000000");
		style.ItemSpacing = { 6.0f, 3.0f };
		style.ItemInnerSpacing = { 6.0f, 4.0f };
		style.IndentSpacing = 16.0f;
		style.TouchExtraPadding = { 0.0f, 0.0f };
		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.5f;
		style.ButtonTextAlign = { 0.5f, 0.5f };
		style.SelectableTextAlign = { 0.0f, 0.5f };

		// Window
		style.WindowPadding = { 6.0f, 6.0f };
		style.WindowMinSize = { 32.0f, 32.0f };
		style.WindowTitleAlign = { 0.0f, 0.5f };
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 1.0f;
		colors[ImGuiCol_WindowBg] = ImGuiUtils::Hex("#242424");
		colors[ImGuiCol_TitleBg] = ImGuiUtils::Hex("#161616");
		colors[ImGuiCol_TitleBgCollapsed] = ImGuiUtils::Hex("#161616BF");
		colors[ImGuiCol_TitleBgActive] = ImGuiUtils::Hex("#0E0E0E");

		// Child Window
		style.ChildRounding = 0.0f;
		style.ChildBorderSize = 1.0f;
		colors[ImGuiCol_ChildBg] = ImGuiUtils::Hex("#00000000");

		// Main Menu Bar
		colors[ImGuiCol_MenuBarBg] = ImGuiUtils::Hex("#161616");

		// Pop-up & Context Menu
		style.PopupRounding = 2.0f;
		style.PopupBorderSize = 1.0f;
		colors[ImGuiCol_PopupBg] = ImGuiUtils::Hex("#1C1C1C");

		// Button
		colors[ImGuiCol_Button] = ImGuiUtils::Hex("#3C3C3C");
		colors[ImGuiCol_ButtonHovered] = ImGuiUtils::Hex("#4A4A4A");
		colors[ImGuiCol_ButtonActive] = ImGuiUtils::Hex("#0E79D0");

		// Frame
		style.FramePadding = { 4.0f, 3.0f };
		style.FrameRounding = 2.0f;
		style.FrameBorderSize = 1.0f;
		colors[ImGuiCol_FrameBg] = ImGuiUtils::Hex("#151515");
		colors[ImGuiCol_FrameBgHovered] = ImGuiUtils::Hex("#1F1F1F");
		colors[ImGuiCol_FrameBgActive] = ImGuiUtils::Hex("#282828");

		// Header
		colors[ImGuiCol_Header] = ImGuiUtils::Hex("#2E2E2E");
		colors[ImGuiCol_HeaderHovered] = ImGuiUtils::Hex("#3A3A3A");
		colors[ImGuiCol_HeaderActive] = ImGuiUtils::Hex("#0E79D0");

		// Scrollbar
		style.ScrollbarSize = 12.0f;
		style.ScrollbarRounding = 0.0f;
		colors[ImGuiCol_ScrollbarBg] = ImGuiUtils::Hex("#1C1C1C");
		colors[ImGuiCol_ScrollbarGrab] = ImGuiUtils::Hex("#5A5A5A");
		colors[ImGuiCol_ScrollbarGrabHovered] = ImGuiUtils::Hex("#6E6E6E");
		colors[ImGuiCol_ScrollbarGrabActive] = ImGuiUtils::Hex("#868686");

		// Slider & Grab
		style.GrabMinSize = 8.0f;
		style.GrabRounding = 2.0f;
		colors[ImGuiCol_SliderGrab] = ImGuiUtils::Hex("#0E79D0");
		colors[ImGuiCol_SliderGrabActive] = ImGuiUtils::Hex("#3D96E0");

		// Checkbox & Radio
		colors[ImGuiCol_CheckMark] = ImGuiUtils::Hex("#0E79D0");

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImGuiUtils::Hex("#00000000");
		colors[ImGuiCol_ResizeGripHovered] = ImGuiUtils::Hex("#4A4A4A");
		colors[ImGuiCol_ResizeGripActive] = ImGuiUtils::Hex("#0E79D0");

		// Tab
		style.TabRounding = 0.0f;
		style.TabBorderSize = 0.0f;
		style.TabBarBorderSize = 1.0f;
		colors[ImGuiCol_Tab] = ImGuiUtils::Hex("#191919");
		colors[ImGuiCol_TabHovered] = ImGuiUtils::Hex("#3C3C3C");
		colors[ImGuiCol_TabSelected] = ImGuiUtils::Hex("#242424");
		colors[ImGuiCol_TabDimmed] = ImGuiUtils::Hex("#141414");
		colors[ImGuiCol_TabDimmedSelected] = ImGuiUtils::Hex("#282828");

		// Separator
		colors[ImGuiCol_Separator] = ImGuiUtils::Hex("#383838");
		colors[ImGuiCol_SeparatorHovered] = ImGuiUtils::Hex("#4A4A4A");
		colors[ImGuiCol_SeparatorActive] = ImGuiUtils::Hex("#0E79D0");

		// Table
		style.CellPadding = { 4.0f, 2.0f };
		colors[ImGuiCol_TableHeaderBg] = ImGuiUtils::Hex("#2E2E2E");
		colors[ImGuiCol_TableBorderStrong] = ImGuiUtils::Hex("#3C3C3C");
		colors[ImGuiCol_TableBorderLight] = ImGuiUtils::Hex("#2A2A2A");
		colors[ImGuiCol_TableRowBg] = ImGuiUtils::Hex("#00000000");
		colors[ImGuiCol_TableRowBgAlt] = ImGuiUtils::Hex("#FFFFFF06");

		// Docking
		colors[ImGuiCol_DockingPreview] = ImGuiUtils::Hex("#0E79D0B2");
		colors[ImGuiCol_DockingEmptyBg] = ImGuiUtils::Hex("#161616");

		// Plot / Graph
		colors[ImGuiCol_PlotLines] = ImGuiUtils::Hex("#FFFFFF");
		colors[ImGuiCol_PlotLinesHovered] = ImGuiUtils::Hex("#3D96E0");
		colors[ImGuiCol_PlotHistogram] = ImGuiUtils::Hex("#0E79D0");
		colors[ImGuiCol_PlotHistogramHovered] = ImGuiUtils::Hex("#3D96E0");

		// Drag & Drop
		colors[ImGuiCol_DragDropTarget] = ImGuiUtils::Hex("#0E79D0E5");

		// Navigation
		colors[ImGuiCol_NavCursor] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		colors[ImGuiCol_NavWindowingHighlight] = ImGuiUtils::Hex("#FFFFFFB2");
		colors[ImGuiCol_NavWindowingDimBg] = ImGuiUtils::Hex("#FFFFFF33");
		colors[ImGuiCol_ModalWindowDimBg] = ImGuiUtils::Hex("#00000099");

		// Shape Rendering
		style.CircleTessellationMaxError = 0.3f;
		style.CurveTessellationTol = 1.25f;
		style.WindowMenuButtonPosition = ImGuiDir_None;
	}
}