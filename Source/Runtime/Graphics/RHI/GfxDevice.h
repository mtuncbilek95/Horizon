#pragma once

#include <Runtime/Graphics/RHI/GfxObject.h>

#include <memory>
#include <functional>
#include <span>

namespace Horizon
{
	struct GfxBufferDesc;
	class GfxBuffer;

	struct GfxTextureDesc;
	class GfxTexture;

	struct GfxGraphicsPipelineDesc;
	struct GfxComputePipelineDesc;
	class GfxPipeline;

	class GfxCommandList;

	struct GfxDeviceDesc
	{
		u32 framesInFlight = 2;
		u32 maxWorkers = 1;
		b8 enableDebug = false;
		b8 enableGPUValidation = false;
	};

	class GfxDevice
	{
	public:
		GfxDevice(const GfxDeviceDesc& desc);
		~GfxDevice();


		GfxHandle GetNative() const { return m_native; }

		std::unique_ptr<GfxBuffer> CreateBuffer(const GfxBufferDesc& desc);
		std::unique_ptr<GfxTexture> CreateTexture(const GfxTextureDesc& desc);
		std::unique_ptr<GfxPipeline> CreatePipeline(const GfxGraphicsPipelineDesc& desc);
		std::unique_ptr<GfxPipeline> CreatePipeline(const GfxComputePipelineDesc& desc);

		void BeginFrame();
		void EndFrame();
		GfxCommandList* AcquireCmd(u32 worker = 0);
		void Submit(GfxQueueType type, std::span<GfxCommandList* const> lists);

		void DeferDestroy(std::function<void()> deleter) const;
		void CollectGarbage() const;
		void WaitIdle() const;

	private:
		GfxHandle m_native;
	};
}