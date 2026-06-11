#pragma once

#include <Engine/Engine/IModule.h>

#include <Runtime/Core/ObjectSlotMap.h>
#include <Runtime/Graphics/GfxBackend.h>

#include <string>
#include <vector>

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
		struct CmdLane
		{
			GfxCmdAllocator* pAllocators[MaxFramesInFlight] = {};
			std::vector<GfxCmdList*> lists[MaxFramesInFlight];
			u32 nextLocal[MaxFramesInFlight] = {};
		};

	public:
		GfxDevice* GetDevice() const { return m_mainDevice; }
		GfxSwapchain* GetSwapchain() const { return m_swapchain; }
		GfxQueue* GetQueue(GfxQueueType type) const;

		GfxTextureHandle CreateTexture(const GfxTextureDesc& desc);
		void DestroyTexture(GfxTextureHandle handl);

		GfxBufferHandle CreateBuffer(const GfxBufferDesc& desc);
		void WriteBuffer(GfxBufferHandle handl, const void* pData, usize sizeInBytes, usize offset);
		void DestroyBuffer(GfxBufferHandle handl);

		GfxPipelineHandle CreatePipeline(const GfxGraphicsPipelineDesc& desc);
		GfxPipelineHandle CreatePipeline(const GfxComputePipelineDesc& desc);
		void DestroyPipeline(GfxPipelineHandle handl);

		GfxTexture* ResolveTexture(GfxTextureHandle handl);
		GfxBuffer* ResolveBuffer(GfxBufferHandle handl);
		GfxPipeline* ResolvePipeline(GfxPipelineHandle handl);

		u32 GetTextureShaderView(GfxTextureHandle handl);
		u32 GetTextureAccessView(GfxTextureHandle handl);
		u32 GetBufferShaderView(GfxBufferHandle handl);
		u32 GetBufferAccessView(GfxBufferHandle handl);

		void BeginFrame();
		void EndFrame();
		GfxTextureHandle GetCurrentBackbuffer();
		u32 GetFrameSlot() const { return u32(m_frameIndex % MaxFramesInFlight); }
		glm::uvec2 GetSurfaceSize() const { return { m_surfaceWidth, m_surfaceHeight }; }

		GfxCmdList* RequestCmdList(GfxQueueType type);
		void SubmitCmdLists(GfxCmdList* const* ppLists, u32 count, GfxQueueType type);

		void OnAttach(Engine& engine) final;
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

		ObjectSlotMap<GfxBuffer*, GfxBufferHandle, 4096> m_bufferPool;
		ObjectSlotMap<GfxTexture*, GfxTextureHandle, 2048> m_texturePool;
		ObjectSlotMap<GfxPipeline*, GfxPipelineHandle, 512> m_pipelinePool;

		std::vector<CmdLane> m_lanes;
		std::vector<GfxTextureHandle> m_backbufferHandles;
		u64 m_slotValues[MaxFramesInFlight] = {};
		u64 m_frameIndex = 0;
		u32 m_threadCount = 1;
		u32 m_surfaceWidth = 0, m_surfaceHeight = 0;
	};
}