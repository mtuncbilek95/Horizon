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
		u32 frameCount = 3;
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
		b8 EndRender(RHI::GfxTexture* backbuffer, u32 imgIndex);

	private:
		void LoadFonts();
		void DefaultStyle();

	private:
		RHI::GfxDevice* m_device;
		RHI::GfxQueue* m_graphicsQueue;
		RHI::GfxDescriptorHeap* m_resourceHeap;

		RenderContext m_context;

		List<RHI::GfxCommandList*> m_commandLists;
	};
}