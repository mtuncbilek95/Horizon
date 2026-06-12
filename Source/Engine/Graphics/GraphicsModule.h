#pragma once

#include <Engine/Core/SlotPool.h>
#include <Engine/Engine/IModule.h>
#include <Engine/Graphics/GraphicsRecord.h>
#include <Engine/Graphics/FrameGraph/FrameGraph.h>

#include <string>
#include <vector>
#include <array>

namespace Horizon
{
	inline constexpr u32 MaxBindlessCapacity = 1 << 16;
	inline constexpr u32 MaxFramesInFlight = 2;
	inline constexpr u32 QueueTypeCount = 3;

	struct GfxBufferTag {};
	using GfxBufferHandle = Handle<GfxBufferTag>;

	struct GfxTextureTag {};
	using GfxTextureHandle = Handle<GfxTextureTag>;

	struct GfxPipelineTag {};
	using GfxPipelineHandle = Handle<GfxPipelineTag>;

	class GraphicsModule : public IModule
	{
	public:
		GfxDevice* GetDevice() const { return m_mainDevice; }
		GfxSwapchain* GetSwapchain() const { return m_swapchain; }
		GfxQueue* GetQueue(GfxQueueType type) const;
		GfxDescriptorHeap* GetDescriptorHeap(GfxDescriptorHeapType type) const;

		GfxTextureHandle CreateTexture(const GfxTextureDesc& desc);
		u32 GetTextureShaderView(GfxTextureHandle handl);
		u32 GetTextureAccessView(GfxTextureHandle handl);
		u32 GetTextureColorView(GfxTextureHandle handl);
		u32 GetTextureDepthView(GfxTextureHandle handl);
		void DestroyTexture(GfxTextureHandle handl);

		GfxCmdList* GetFrameCmd() const { return m_frameCmd; }
		GfxTexture* GetCurrentPresentImage() const { return m_currPresentImg; }
		GfxTexture* GetTexture(GfxTextureHandle handl);

		void BeginFrame();
		void RenderScene();
		void EndFrame();

		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
		GfxDevice* m_mainDevice = nullptr;
		GfxSwapchain* m_swapchain = nullptr;

		GfxDescriptorHeap* m_resourceHeap = nullptr;
		GfxDescriptorHeap* m_colorHeap = nullptr;
		GfxDescriptorHeap* m_depthHeap = nullptr;

		GfxQueue* m_graphicsQueue = nullptr;
		GfxQueue* m_computeQueue = nullptr;
		GfxQueue* m_transferQueue = nullptr;

		GfxPipelineLayout* m_globalLayout = nullptr;

		GfxCmdAllocator* m_frameAllocators[MaxFramesInFlight];
		GfxCmdList* m_frameCmd = nullptr;
		GfxTexture* m_currPresentImg = nullptr;

		u64 m_frameFenceValues[MaxFramesInFlight] = {};
		u64 m_frameIndex = 0;

		// Pools
	private:
		SlotPool<TextureRecord, GfxTextureHandle> m_texturePool;

	private:
		FrameGraph m_graph;
	};
}