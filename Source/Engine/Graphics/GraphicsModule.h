#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/Graphics/GraphicsPool.h>
#include <Engine/Graphics/GraphicsRecord.h>

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

		GfxTextureHandle CreateTexture(const GfxTextureDesc& desc);
		void DestroyTexture(GfxTextureHandle handl);

		GfxBufferHandle CreateBuffer(const GfxBufferDesc& desc);
		void WriteBuffer(GfxBufferHandle handl, const void* pData, usize sizeInBytes, usize offset);
		void DestroyBuffer(GfxBufferHandle handl);

		GfxPipelineHandle CreatePipeline(const GfxGraphicsPipelineDesc& desc);
		GfxPipelineHandle CreatePipeline(const GfxComputePipelineDesc& desc);
		void DestroyPipeline(GfxPipelineHandle handl);

		u32 GetTextureShaderView(GfxTextureHandle handl);
		u32 GetTextureAccessView(GfxTextureHandle handl);
		u32 GetBufferShaderView(GfxBufferHandle handl);
		u32 GetBufferAccessView(GfxBufferHandle handl);

		void BeginFrame();
		void EndFrame();

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

		GraphicsPool<TextureRecord, GfxTextureTag> m_textures;
		GraphicsPool<BufferRecord, GfxBufferTag> m_buffer;
		GraphicsPool<PipelineRecord, GfxPipelineTag> m_pipelines;
	};
}