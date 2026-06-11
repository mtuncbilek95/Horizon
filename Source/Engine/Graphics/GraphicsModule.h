#pragma once

#include <Engine/Engine/IModule.h>

#include <Runtime/Core/ObjectSlotMap.h>
#include <Runtime/Graphics/GfxBackend.h>

#include <string>
#include <vector>

namespace Horizon
{
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

		GfxTextureHandle CreateTexture(const GfxTextureDesc& desc);
		void DestroyTexture(GfxTextureHandle handl);

		GfxBufferHandle CreateBuffer(const GfxBufferDesc& desc);
		void DestroyBuffer(GfxBufferHandle handl);

		GfxPipelineHandle CreatePipeline(const GfxGraphicsPipelineDesc& desc);
		GfxPipelineHandle CreatePipeline(const GfxComputePipelineDesc& desc);
		void DestroyPipeline(GfxPipelineHandle handl);

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
	};
}