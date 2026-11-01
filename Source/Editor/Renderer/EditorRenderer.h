#pragma once

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Command/CommandPool.h>
#include <Runtime/RHI/Fence/Fence.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::RHI
{
	class Device;
	class DescriptorHeap;
	class Swapchain;
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
		RHI::Device* pDevice = nullptr;
		RHI::DescriptorHeap* pResourceHeap = nullptr;
		RHI::DescriptorHeap* pSamplerHeap = nullptr;
		RHI::TextureFormat colorFormat = RHI::TextureFormat::BGRA8_UNORM;
		Engine::WindowService* pWindowService = nullptr;
	};

	class EDITOR_API EditorRenderer
	{
		struct FrameContext
		{
			UniquePtr<RHI::CommandPool> pPool;
			UniquePtr<RHI::CommandList> pCmdList;
			u64 fenceValue = 0;
		};

	public:
		EditorRenderer(const EditorRendererDesc& desc);
		~EditorRenderer();

		void UpdatePlatform();

		b8 BeginRender(f32 dt);
		b8 EndRender(RHI::Swapchain* pSwapchain);
		b8 CheckMouseDragging();

	private:
		void LoadFonts();
		void DefaultStyle();

	private:
		RenderContext m_context;
		UniquePtr<EditorPlatform> m_platform = nullptr;

		RHI::Device* m_device = nullptr;
		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		RHI::DescriptorHeap* m_samplerHeap = nullptr;
		List<FrameContext> m_frames;

		UniquePtr<RHI::Fence> m_fence;
		u32 m_frameIndex = 0;
	};
}
