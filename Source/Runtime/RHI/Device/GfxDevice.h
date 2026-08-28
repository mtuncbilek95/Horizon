#pragma once

#include <Runtime/RHI/Device/GfxDeviceDesc.h>
#include <Runtime/RHI/Queue/GfxQueueType.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::RHI
{
	struct GfxDescriptorHeapDesc;
	class GfxDescriptorHeap;

	struct GfxSwapchainDesc;
	class GfxSwapchain;

	struct GfxTextureDesc;
	class GfxTexture;

	struct GfxBufferDesc;
	class GfxBuffer;

	struct GfxBufferArenaDesc;
	class GfxBufferArena;

	struct GfxUploadRingDesc;
	class GfxUploadRing;

	struct GfxShaderDesc;
	class GfxShader;

	struct GfxGraphicsPipelineDesc;
	struct GfxComputePipelineDesc;
	class GfxPipeline;

	class GfxCommandList;
	class GfxQueue;
	class GfxFence;

	/**
	 * @brief Logical graphics device and the factory of
	 * every other RHI object. Owns the backend and stays
	 * alive as long as any resource it created.
	 *
	 * @code
	 *   RHI::GfxDeviceDesc devDesc = {};
	 *   RHI::GfxDevice* device = RHI::CreateDevice(devDesc);
	 *   RHI::GfxQueue* queue = device->CreateQueue(RHI::GfxQueueType::Graphics);
	 * @endcode
	 */
	class H_EXPORT GfxDevice
	{
	public:
		virtual ~GfxDevice() = default;

		virtual void InitializeImGui(u32 maxFrames, GfxQueue* pQueue, GfxDescriptorHeap* pHeap, GfxTextureFormat colorFormat) = 0;
		virtual void NewFrameImGui() = 0;
		virtual void ShutdownImGui() = 0;

		virtual GfxDescriptorHeap* CreateDescriptorHeap(const GfxDescriptorHeapDesc& desc) = 0;
		virtual GfxSwapchain* CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* pPresentQueue) = 0;

		virtual GfxTexture* CreateTexture(const GfxTextureDesc& desc) = 0;
		virtual GfxBuffer* CreateBuffer(const GfxBufferDesc& desc) = 0;
		virtual GfxBufferArena* CreateBufferArena(const GfxBufferArenaDesc& desc) = 0;
		virtual GfxUploadRing* CreateUploadRing(const GfxUploadRingDesc& desc) = 0;
		virtual GfxShader* CreateShader(const GfxShaderDesc& desc) = 0;
		virtual GfxPipeline* CreatePipeline(const GfxGraphicsPipelineDesc& desc) = 0;
		virtual GfxPipeline* CreatePipeline(const GfxComputePipelineDesc& desc) = 0;

		virtual GfxCommandList* CreateCommandList(GfxQueueType type) = 0;
		virtual GfxQueue* CreateQueue(GfxQueueType type) = 0;
		virtual GfxFence* CreateFence() = 0;

		virtual void WaitIdle() = 0;
	};

	/**
	 * @brief Instantiates the graphics device of the
	 * backend this build was compiled against.
	 *
	 * @param desc Debug and validation flags of the device.
	 * @return The new device, or nullptr when no supported
	 *         adapter was found.
	 */
	GfxDevice* CreateDevice(const GfxDeviceDesc& desc);
}