#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>

#include <glm/glm.hpp>

#include <span>
#include <string_view>

namespace Horizon
{
	struct GfxBufferTag {};
	using GfxBufferHandle = Handle<GfxBufferTag>;

	struct GfxTextureTag {};
	using GfxTextureHandle = Handle<GfxTextureTag>;

	struct GfxPipelineTag {};
	using GfxPipelineHandle = Handle<GfxPipelineTag>;

	struct GfxCmdListTag {};
	using GfxCmdListHandle = Handle<GfxCmdListTag>;

	struct GfxFenceTag {};
	using GfxFenceHandle = Handle<GfxFenceTag>;

	struct GfxBufferDesc
	{
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GPU;
		usize size = 0;
		u32 stride = 0;
		std::string_view debugName = "";
	};

	struct GfxTextureDesc
	{};

	struct GfxGraphicsPipelineDesc
	{};

	struct GfxComputePipelineDesc
	{};

	struct GfxSwapchainDesc
	{};

	struct GfxFenceDesc
	{
		u64 initialValue = 0;
		std::string_view debugName = "";
	};

	struct GfxBufferBarrier
	{
		GfxBufferHandle buffer;
		GfxResourceState before;
		GfxResourceState after;
	};

	struct GfxTextureBarrier
	{
		GfxTextureHandle buffer;
		GfxResourceState before;
		GfxResourceState after;
	};

	struct GfxTextureCopyDesc
	{};

	struct GfxCommandListDesc
	{
		GfxQueueType queue = GfxQueueType::Graphics;
		u32 workerIndex = 0;
	};

	struct GfxRenderBeginDesc
	{};

	struct GfxDeviceDesc
	{
		b8 enableDebug = false;
		b8 enableGPUValidation = false;
		u32 cmdWorkerCount = 1;
	};

	namespace GfxDevice
	{
		void InitializeDevice(const GfxDeviceDesc& desc);
		void ShutdownDevice();

		GfxBufferHandle CreateBuffer(const GfxBufferDesc& desc);
		void WriteBuffer(GfxBufferHandle handle, void* pData, usize sizeInBytes, usize offset);
		void DestroyBuffer(GfxBufferHandle handle);

		GfxTextureHandle CreateTexture(const GfxTextureDesc& desc);
		void WriteTexture(GfxTextureHandle handle, void* pData, usize sizeInBytes, usize offset);
		void DestroyTexture(GfxTextureHandle handle);

		GfxPipelineHandle CreatePipeline(const GfxGraphicsPipelineDesc& desc);
		GfxPipelineHandle CreatePipeline(const GfxComputePipelineDesc& desc);
		void DestroyPipeline(GfxPipelineHandle handle);

		GfxCmdListHandle CreateCommandList(const GfxCommandListDesc& desc);
		void DestroyFence(GfxFenceHandle handle);

		GfxFenceHandle CreateFence(const GfxFenceDesc& desc);

		void FlushUploads();
		void WaitIdle();
	}

	namespace GfxFence
	{
		u64 GetCompletedValue(GfxFenceHandle handle);
		void WaitForCPU(GfxFenceHandle handle, u64 value);
		void Signal(GfxFenceHandle handle, u64 value);
	}

	namespace GfxCmdList
	{
		void BeginRendering(GfxCmdListHandle command, const GfxRenderBeginDesc& desc);
		void EndRendering(GfxCmdListHandle command);

		void BeginMarker(GfxCmdListHandle command, std::string_view name, const glm::uvec3& color);
		void EndMarker(GfxCmdListHandle command);

		void BindPipeline(GfxCmdListHandle command, GfxPipelineHandle handle);

		void Draw(GfxCmdListHandle command, u32 vertexCount, u32 instanceCount);
		void DrawIndexed(GfxCmdListHandle command, u32 indexCount, u32 instanceCount, u32 firstIndex = 0, i32 vertexOffset = 0);

		void Dispatch(GfxCmdListHandle command, u32 groupCountX, u32 groupCountY, u32 groupCountZ);
		void DispatchMesh(GfxCmdListHandle command, u32 groupCountX, u32 groupCountY, u32 groupCountZ);

		void Barrier(GfxCmdListHandle command, std::span<GfxTextureBarrier> barriers);
		void Barrier(GfxCmdListHandle command, std::span<GfxBufferBarrier> barriers);

		void UploadBuffer(GfxCmdListHandle command, GfxBufferHandle src, usize srcOffset, GfxBufferHandle dst, usize dstOffset, usize sizeInBytes);
		void UploadTexture(GfxCmdListHandle command, GfxBufferHandle src, usize srcOffset, GfxTextureHandle dst, usize dstOffset, const GfxTextureCopyDesc& region);
	}
}