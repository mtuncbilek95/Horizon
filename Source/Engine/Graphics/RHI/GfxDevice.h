#pragma once

#include <Engine/Graphics/RHI/GfxTypes.h>

namespace Horizon
{
	struct GfxBufferTag {};
	using GfxBufferHandle = Handle<GfxBufferTag>;

	struct GfxTextureTag {};
	using GfxTextureHandle = Handle<GfxTextureTag>;

	struct GfxSamplerTag {};
	using GfxSamplerHandle = Handle<GfxSamplerTag>;

	struct GfxPipelineTag {};
	using GfxPipelineHandle = Handle<GfxPipelineTag>;

	struct GfxCommandSignatureTag {};
	using GfxCommandSignatureHandle = Handle<GfxCommandSignatureTag>;

	struct GfxCommandList;

	struct GfxBufferDesc;
	struct GfxTextureDesc;
	struct GfxSamplerDesc;
	struct GfxGraphicsPipelineDesc;
	struct GfxComputePipelineDesc;
	struct GfxCommandSignatureDesc;

	struct GfxSemaphore
	{
		u64 value;
		void* handle = nullptr;
	};

	struct GfxDeviceDesc
	{
		void* windowHandle = nullptr;
		u32 framesInFlight = 2;
		u32 maxWorkers = 1;
		b8 enableDebug = false;
	};

	namespace GfxDevice
	{
		void Initialize(const GfxDeviceDesc& desc);
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		GfxBufferHandle CreateBuffer(const GfxBufferDesc& desc);
		void WriteBuffer(GfxBufferHandle handle, const void* data, usize size, usize offset = 0);
		void UploadBuffer(GfxBufferHandle handle, const void* data, usize size, usize offset = 0);
		void DestroyBuffer(GfxBufferHandle handle);

		GfxTextureHandle CreateTexture(const GfxTextureDesc& desc);
		void UploadTexture(GfxTextureHandle handle, const void* data, u32 mip = 0, u32 slice = 0);
		void DestroyTexture(GfxTextureHandle handle);

		GfxPipelineHandle CreatePipeline(const GfxGraphicsPipelineDesc& desc);
		GfxPipelineHandle CreatePipeline(const GfxComputePipelineDesc& desc);
		void DestroyPipeline(GfxPipelineHandle handle);

		GfxCommandSignatureHandle CreateCommandSignature(const GfxCommandSignatureDesc& desc);
		void DestroyCommandSignature(GfxCommandSignatureHandle handle);
		u32 CommandSignatureStride(GfxCommandSignatureHandle handle);

		GfxSamplerHandle CreateSampler(const GfxSamplerDesc& desc);
		void DestroySampler(GfxSamplerHandle handle);

		u32 BindlessShaderView(GfxBufferHandle handle);
		u32 BindlessAccessView(GfxBufferHandle handle);

		u32 BindlessShaderView(GfxTextureHandle handle);
		u32 BindlessAccessView(GfxTextureHandle handle);

		GfxCommandList* AcquireCmd(GfxQueueType type, u32 workerIndex);

		GfxSemaphore CreateSync();
		void DestroySemaphore(GfxSemaphore& semaphore);

		u64 SemaphoreValue(const GfxSemaphore& semaphore);
		void WaitSemaphore(const GfxSemaphore& semaphore, u64 value);
		void SignalSemaphore(const GfxSemaphore& semaphore, u64 value);

		void WaitIdle();
	}
}
