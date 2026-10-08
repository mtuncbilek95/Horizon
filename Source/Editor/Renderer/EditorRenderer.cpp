#include "EditorRenderer.h"

#include <Editor/Font/IconsFontAudio.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Font/IconsKenney.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Engine/Graphics/GraphicsContext.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/RHI/Texture/GfxTexture.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <backends/imgui_impl_dx12.h>

namespace Horizon::Editor
{
	EditorRenderer::EditorRenderer(const EditorRendererDesc& desc) : m_device(desc.pDevice),
		m_graphicsQueue(desc.pQueue)
	{
		m_context = ImGui::CreateContext();
		ImGui::SetCurrentContext((ImGuiContext*)m_context);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags = ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
		io.DisplaySize = { 512.f, 512.f };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };

		LoadFonts();

		RHI::GfxDescriptorHeapDesc heapDesc = {};
		heapDesc.type = RHI::GfxDescriptorHeapType::Resource;
		heapDesc.capacity = kImGuiHeapCapacity;

		m_resourceHeap = m_device->CreateDescriptorHeap(heapDesc);

		if (m_resourceHeap == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "ImGui resource heap with capacity {} could not be created", kImGuiHeapCapacity);
			return;
		}

		m_device->InitializeImGui(Engine::GraphicsContext::MaxFramesInFlight, m_graphicsQueue, m_resourceHeap, desc.colorFormat);

		m_frames.Resize(Engine::GraphicsContext::MaxFramesInFlight);
		for (u32 i = 0; i < Engine::GraphicsContext::MaxFramesInFlight; i++)
			m_frames[i].pCmdList = m_device->CreateCommandList(RHI::GfxQueueType::Graphics);

		m_fence = m_device->CreateFence();

		DefaultStyle();
	}

	EditorRenderer::~EditorRenderer()
	{
		m_device->WaitIdle();

		for (auto& frame : m_frames)
			Memory::Allocator::Delete(frame.pCmdList);

		Memory::Allocator::Delete(m_fence);
		m_device->ShutdownImGui();

		Memory::Allocator::Delete(m_resourceHeap);

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
		io.AddMouseWheelEvent(0.0f, delta);
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

	void EditorRenderer::OnWindowFocus(b8 focused)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddFocusEvent(focused);
	}

	b8 EditorRenderer::BeginRender(f32 dt)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;

		m_device->NewFrameImGui();
		ImGui::NewFrame();

		return true;
	}

	b8 EditorRenderer::EndRender(RHI::GfxTexture* pBackbuffer)
	{
		ImGui::Render();

		FrameContext& context = m_frames[m_frameIndex];

		m_fence->WaitCPU(context.fenceValue);

		context.pCmdList->Begin();
		context.pCmdList->BindDescriptorHeaps(m_resourceHeap, nullptr);

		RHI::GfxTextureBarrier toTarget = { pBackbuffer, RHI::GfxResourceState::Present, RHI::GfxResourceState::RenderTarget };
		context.pCmdList->Barrier(&toTarget, 1);

		const RHI::GfxTextureDesc& bbDesc = pBackbuffer->GetDesc();
		RHI::GfxRenderBeginDesc pass = {};

		pass.AddColorTarget(pBackbuffer, RHI::GfxLoadOp::Clear, { 0.1f, 0.1f, 0.1f, 1.0f })
			.SetSize(bbDesc.width, bbDesc.height);
		context.pCmdList->BeginRendering(pass);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), (ID3D12GraphicsCommandList6*)context.pCmdList->GetAPIHandle());

		context.pCmdList->EndRendering();

		RHI::GfxTextureBarrier toPresent = { pBackbuffer, RHI::GfxResourceState::RenderTarget, RHI::GfxResourceState::Present };

		context.pCmdList->Barrier(&toPresent, 1);

		context.pCmdList->End();
		m_graphicsQueue->Submit(&context.pCmdList, 1);

		context.fenceValue = m_graphicsQueue->Signal(m_fence);
		m_frameIndex = (m_frameIndex + 1) % Engine::GraphicsContext::MaxFramesInFlight;

		return true;
	}

	b8 EditorRenderer::CheckMouseDragging()
	{
		return ImGui::IsAnyItemActive() && ImGui::IsAnyMouseDown();
	}

	void EditorRenderer::LoadFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		const std::string fontDir = std::string(HORIZON_RESOURCE_DIR) + "/Fonts/";
		constexpr f32 fontSize = 16.0f;

		static const ImWchar faRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		static const ImWchar kiRange[] = { ICON_MIN_KI, ICON_MAX_KI, 0 };

		const auto mergeIconFont = [&](const char* pFile, const ImWchar* pRange)
			{
				ImFontConfig iconCfg = {};
				iconCfg.MergeMode = true;
				iconCfg.PixelSnapH = true;
				iconCfg.ExtraSizeScale = 0.85f;
				iconCfg.GlyphMinAdvanceX = fontSize;

				const std::string path = fontDir + pFile;
				if (!io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize, &iconCfg, pRange))
					Terminal::Error(StringOps::GetName(this), "Failed to load icon font: {}", path);
			};

		const auto loadFamily = [&](const char* pFile) -> ImFont*
			{
				const std::string path = fontDir + pFile;
				ImFont* pFont = io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize);

				if (pFont == nullptr)
				{
					Terminal::Error(StringOps::GetName(this), "Failed to load UI Font: {}", path);
					pFont = io.Fonts->AddFontDefault();
				}

				mergeIconFont("fa-solid-900.ttf", faRange);
				mergeIconFont("kenney-icon-font.ttf", kiRange);

				return pFont;
			};

		io.FontDefault = loadFamily("NotoSans-Medium.ttf");
		loadFamily("NotoSans-SemiBold.ttf");
	}

	void EditorRenderer::DefaultStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* pColors = style.Colors;

		pColors[ImGuiCol_Text] = ImGuiUtils::Hex("#FFFFFF");
		pColors[ImGuiCol_TextDisabled] = ImGuiUtils::Hex("#808080");
		pColors[ImGuiCol_TextSelectedBg] = ImGuiUtils::Hex("#0E79D0");
		pColors[ImGuiCol_Border] = ImGuiUtils::Hex("#383838");
		pColors[ImGuiCol_BorderShadow] = ImGuiUtils::Hex("#00000000");
		style.ItemSpacing = { 6.0f, 3.0f };
		style.ItemInnerSpacing = { 6.0f, 4.0f };
		style.IndentSpacing = 16.0f;
		style.TouchExtraPadding = { 0.0f, 0.0f };
		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.5f;
		style.ButtonTextAlign = { 0.5f, 0.5f };
		style.SelectableTextAlign = { 0.0f, 0.5f };

		style.WindowPadding = { 6.0f, 6.0f };
		style.WindowMinSize = { 32.0f, 32.0f };
		style.WindowTitleAlign = { 0.0f, 0.5f };
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 1.0f;
		pColors[ImGuiCol_WindowBg] = ImGuiUtils::Hex("#242424");
		pColors[ImGuiCol_TitleBg] = ImGuiUtils::Hex("#161616");
		pColors[ImGuiCol_TitleBgCollapsed] = ImGuiUtils::Hex("#161616BF");
		pColors[ImGuiCol_TitleBgActive] = ImGuiUtils::Hex("#0E0E0E");

		style.ChildRounding = 0.0f;
		style.ChildBorderSize = 1.0f;
		pColors[ImGuiCol_ChildBg] = ImGuiUtils::Hex("#00000000");

		pColors[ImGuiCol_MenuBarBg] = ImGuiUtils::Hex("#161616");

		style.PopupRounding = 2.0f;
		style.PopupBorderSize = 1.0f;
		pColors[ImGuiCol_PopupBg] = ImGuiUtils::Hex("#1C1C1C");

		pColors[ImGuiCol_Button] = ImGuiUtils::Hex("#3C3C3C");
		pColors[ImGuiCol_ButtonHovered] = ImGuiUtils::Hex("#4A4A4A");
		pColors[ImGuiCol_ButtonActive] = ImGuiUtils::Hex("#0E79D0");

		style.FramePadding = { 4.0f, 3.0f };
		style.FrameRounding = 2.0f;
		style.FrameBorderSize = 1.0f;
		pColors[ImGuiCol_FrameBg] = ImGuiUtils::Hex("#151515");
		pColors[ImGuiCol_FrameBgHovered] = ImGuiUtils::Hex("#1F1F1F");
		pColors[ImGuiCol_FrameBgActive] = ImGuiUtils::Hex("#282828");

		pColors[ImGuiCol_Header] = ImGuiUtils::Hex("#2E2E2E");
		pColors[ImGuiCol_HeaderHovered] = ImGuiUtils::Hex("#3A3A3A");
		pColors[ImGuiCol_HeaderActive] = ImGuiUtils::Hex("#0E79D0");

		style.ScrollbarSize = 12.0f;
		style.ScrollbarRounding = 0.0f;
		pColors[ImGuiCol_ScrollbarBg] = ImGuiUtils::Hex("#1C1C1C");
		pColors[ImGuiCol_ScrollbarGrab] = ImGuiUtils::Hex("#5A5A5A");
		pColors[ImGuiCol_ScrollbarGrabHovered] = ImGuiUtils::Hex("#6E6E6E");
		pColors[ImGuiCol_ScrollbarGrabActive] = ImGuiUtils::Hex("#868686");

		style.GrabMinSize = 8.0f;
		style.GrabRounding = 2.0f;
		pColors[ImGuiCol_SliderGrab] = ImGuiUtils::Hex("#0E79D0");
		pColors[ImGuiCol_SliderGrabActive] = ImGuiUtils::Hex("#3D96E0");

		pColors[ImGuiCol_CheckMark] = ImGuiUtils::Hex("#0E79D0");

		pColors[ImGuiCol_ResizeGrip] = ImGuiUtils::Hex("#00000000");
		pColors[ImGuiCol_ResizeGripHovered] = ImGuiUtils::Hex("#4A4A4A");
		pColors[ImGuiCol_ResizeGripActive] = ImGuiUtils::Hex("#0E79D0");

		style.TabRounding = 0.0f;
		style.TabBorderSize = 0.0f;
		style.TabBarBorderSize = 1.0f;
		pColors[ImGuiCol_Tab] = ImGuiUtils::Hex("#191919");
		pColors[ImGuiCol_TabHovered] = ImGuiUtils::Hex("#3C3C3C");
		pColors[ImGuiCol_TabSelected] = ImGuiUtils::Hex("#242424");
		pColors[ImGuiCol_TabDimmed] = ImGuiUtils::Hex("#141414");
		pColors[ImGuiCol_TabDimmedSelected] = ImGuiUtils::Hex("#282828");

		pColors[ImGuiCol_Separator] = ImGuiUtils::Hex("#383838");
		pColors[ImGuiCol_SeparatorHovered] = ImGuiUtils::Hex("#4A4A4A");
		pColors[ImGuiCol_SeparatorActive] = ImGuiUtils::Hex("#0E79D0");

		style.CellPadding = { 4.0f, 2.0f };
		pColors[ImGuiCol_TableHeaderBg] = ImGuiUtils::Hex("#2E2E2E");
		pColors[ImGuiCol_TableBorderStrong] = ImGuiUtils::Hex("#3C3C3C");
		pColors[ImGuiCol_TableBorderLight] = ImGuiUtils::Hex("#2A2A2A");
		pColors[ImGuiCol_TableRowBg] = ImGuiUtils::Hex("#00000000");
		pColors[ImGuiCol_TableRowBgAlt] = ImGuiUtils::Hex("#FFFFFF06");

		pColors[ImGuiCol_DockingPreview] = ImGuiUtils::Hex("#0E79D0B2");
		pColors[ImGuiCol_DockingEmptyBg] = ImGuiUtils::Hex("#161616");

		pColors[ImGuiCol_PlotLines] = ImGuiUtils::Hex("#FFFFFF");
		pColors[ImGuiCol_PlotLinesHovered] = ImGuiUtils::Hex("#3D96E0");
		pColors[ImGuiCol_PlotHistogram] = ImGuiUtils::Hex("#0E79D0");
		pColors[ImGuiCol_PlotHistogramHovered] = ImGuiUtils::Hex("#3D96E0");

		pColors[ImGuiCol_DragDropTarget] = ImGuiUtils::Hex("#0E79D0E5");

		pColors[ImGuiCol_NavCursor] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		pColors[ImGuiCol_NavWindowingHighlight] = ImGuiUtils::Hex("#FFFFFFB2");
		pColors[ImGuiCol_NavWindowingDimBg] = ImGuiUtils::Hex("#FFFFFF33");
		pColors[ImGuiCol_ModalWindowDimBg] = ImGuiUtils::Hex("#00000099");

		style.CircleTessellationMaxError = 0.3f;
		style.CurveTessellationTol = 1.25f;
		style.WindowMenuButtonPosition = ImGuiDir_None;
	}

	PAL::CursorType EditorRenderer::GetMouseCursor() const
	{
		const ImGuiIO& io = ImGui::GetIO();

		if (io.MouseDrawCursor)
			return PAL::CursorType::Hidden;

		return ImGuiUtils::ToCursorType(ImGui::GetMouseCursor());
	}
}