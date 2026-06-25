#pragma once

#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/PAL/Window/KeyCode.h>

#include <Runtime/RHI/GfxTypes.h>

namespace Horizon
{
	using RenderContext = void*;

	class GfxDevice;
	class GfxQueue;
	class GfxTexture;
	class GfxPipeline;
	class GfxBuffer;
	class GfxCommandList;
	class GfxFence;

	struct EditorRendererDesc
	{
		GfxDevice* pDevice = nullptr;
		GfxQueue* pQueue = nullptr;
	};

	class H_EXPORT EditorRenderer
	{
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
		b8 EndRender(GfxTexture* backbuffer);

	private:
		GfxDevice* m_device;
		GfxQueue* m_graphicsQueue;

		RenderContext m_context;

		GfxPipeline* m_pipeline = nullptr;
		GfxTexture* m_fontTexture = nullptr;

		GfxCommandList* m_commandLists[MaxFramesInFlight] = {};
		GfxBuffer* m_vertexBuffers[MaxFramesInFlight] = {};
		GfxBuffer* m_indexBuffers[MaxFramesInFlight] = {};
		u32 m_vertexCapacities[MaxFramesInFlight] = {};
		u32 m_indexCapacities[MaxFramesInFlight] = {};

		GfxFence* m_fence = nullptr;
		u64 m_frameFenceValues[MaxFramesInFlight] = {};
		u32 m_frameIndex = 0;
	};
}