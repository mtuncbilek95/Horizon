#pragma once

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

namespace Horizon::Engine
{
	class WindowService;
}

namespace Horizon::Editor
{
	class EditorPlatform;

	using RenderContext = void*;

	struct EditorRendererDesc
	{
		RHI::GfxDevice* pDevice = nullptr;
		RHI::GfxQueue* pQueue = nullptr;
		RHI::GfxDescriptorHeap* pResourceHeap = nullptr;
		RHI::GfxTextureFormat colorFormat = RHI::GfxTextureFormat::RGBA8_UNORM;
		Engine::WindowService* pWindowService = nullptr;
	};

	class EDITOR_API EditorRenderer
	{
		struct FrameContext
		{
			RHI::GfxCommandList* pCmdList = nullptr;
			u32 fenceValue = 0;
		};

	public:
		EditorRenderer(const EditorRendererDesc& desc);
		~EditorRenderer();

		void UpdatePlatform();

		b8 BeginRender(f32 dt);
		b8 EndRender(RHI::GfxTexture* pBackbuffer);
		b8 CheckMouseDragging();

	private:
		void LoadFonts();
		void DefaultStyle();

	private:
		RenderContext m_context;
		EditorPlatform* m_platform = nullptr;

		RHI::GfxDevice* m_device;
		RHI::GfxQueue* m_graphicsQueue;
		RHI::GfxDescriptorHeap* m_resourceHeap = nullptr;
		List<FrameContext> m_frames;

		RHI::GfxFence* m_fence = nullptr;
		u32 m_frameIndex = 0;
	};
}