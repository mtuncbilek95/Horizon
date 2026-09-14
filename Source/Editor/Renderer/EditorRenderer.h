#pragma once

#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::RHI
{
	class GfxDevice;
	class GfxQueue;
	class GfxTexture;
	class GfxPipeline;
	class GfxBuffer;
	class GfxCommandList;
	class GfxDescriptorHeap;
	class GfxFence;
}

namespace Horizon::Editor
{
	using RenderContext = void*;

	struct EditorRendererDesc
	{
		RHI::GfxDevice* pDevice = nullptr;
		RHI::GfxQueue* pQueue = nullptr;
		RHI::GfxDescriptorHeap* pResourceHeap = nullptr;
		RHI::GfxTextureFormat colorFormat = RHI::GfxTextureFormat::RGBA8_UNORM;
	};

	class H_EXPORT EditorRenderer
	{
		struct FrameContext
		{
			RHI::GfxCommandList* pCmdList = nullptr;
			u32 fenceValue = 0;
		};

	public:
		EditorRenderer(const EditorRendererDesc& desc);
		~EditorRenderer();

		void OnMousePosition(i32 x, i32 y);
		void OnMouseButtonDown(PAL::MouseButton button);
		void OnMouseButtonUp(PAL::MouseButton button);
		void OnMouseWheel(f32 delta);
		void OnKeyboardDown(PAL::KeyCode key);
		void OnKeyboardUp(PAL::KeyCode key);
		void OnKeyboardChar(u32 value);
		void OnResizeWindow(u32 width, u32 height);

		b8 BeginRender(f32 dt);
		b8 EndRender(RHI::GfxTexture* backbuffer);

	private:
		void LoadFonts();
		void DefaultStyle();

	private:
		RenderContext m_context;

		RHI::GfxDevice* m_device;
		RHI::GfxQueue* m_graphicsQueue;
		RHI::GfxDescriptorHeap* m_resourceHeap;
		List<FrameContext> m_frames;
		
		RHI::GfxFence* m_fence;
		u32 m_frameIndex = 0;
	};
}