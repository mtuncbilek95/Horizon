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

#include <fstream>
#include <vector>

namespace Horizon
{
	namespace
	{
		struct ImGuiVertexPush
		{
			f32 scale[2];
			f32 translate[2];
			u32 vertexBufferIndex;
			u32 textureIndex;
		};

		std::vector<u8> ReadShaderFile(const c8* path)
		{
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<u8> buffer(static_cast<usize>(size));
			file.read((c8*)buffer.data(), size);

			return buffer;
		}
	}

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

		m_fence = m_device->CreateFence();

		CreatePipeline();
	}

	EditorRenderer::~EditorRenderer()
	{
		if (m_fence)
		{
			for (u32 i = 0; i < MaxFramesInFlight; i++)
				m_fence->WaitCPU(m_frameFenceValues[i]);
		}

		Allocator::Delete(m_pipeline);
		Allocator::Delete(m_fence);

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

		ImGui::NewFrame();

		return true;
	}

	b8 EditorRenderer::EndRender(GfxTexture* backbuffer)
	{
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		const u32 slot = m_frameIndex % MaxFramesInFlight;
		m_fence->WaitCPU(m_frameFenceValues[slot]);

		const u32 totalVtx = u32(drawData->TotalVtxCount);
		const u32 totalIdx = u32(drawData->TotalIdxCount);

		m_graphicsQueue->Submit(nullptr, 0);
		m_frameFenceValues[slot] = m_graphicsQueue->Signal(m_fence);

		m_frameIndex++;
		return true;
	}

	void EditorRenderer::CreatePipeline()
	{
		std::vector<u8> vertexBytes = ReadShaderFile("ImGui.vert");
		std::vector<u8> pixelBytes = ReadShaderFile("ImGui.frag");

		GfxGraphicsPipelineDesc pipeDesc = {};
		pipeDesc.vertexShader = { vertexBytes.data(), vertexBytes.size() };
		pipeDesc.pixelShader = { pixelBytes.data(), pixelBytes.size() };
		pipeDesc.colorFormats[0] = GfxTextureFormat::RGBA8;
		pipeDesc.colorTargetCount = 1;
		pipeDesc.depthFormat = GfxTextureFormat::Undefined;
		pipeDesc.topology = GfxPrimitiveTopology::TriangleList;
		pipeDesc.cullMode = GfxCullMode::None;
		pipeDesc.fillMode = GfxFillMode::Solid;
		pipeDesc.depthTest = false;
		pipeDesc.depthWrite = false;

		pipeDesc.blend.enable = true;
		pipeDesc.blend.srcColor = GfxBlendFactor::SrcAlpha;
		pipeDesc.blend.dstColor = GfxBlendFactor::InvSrcAlpha;
		pipeDesc.blend.colorOp = GfxBlendOp::Add;
		pipeDesc.blend.srcAlpha = GfxBlendFactor::One;
		pipeDesc.blend.dstAlpha = GfxBlendFactor::InvSrcAlpha;
		pipeDesc.blend.alphaOp = GfxBlendOp::Add;
		pipeDesc.blend.writeMask = GfxColorWrite::All;

		m_pipeline = m_device->CreatePipeline(pipeDesc);
	}
}