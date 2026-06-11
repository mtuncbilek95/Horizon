#pragma once

#include <Runtime/Graphics/GfxTypes.h>

#include <glm/glm.hpp>

namespace Horizon
{
	struct GfxDevice;
	struct GfxSwapchain;
	struct GfxQueue;
	struct GfxBuffer;
	struct GfxTexture;
	struct GfxPipelineLayout;
	struct GfxPipeline;
	struct GfxCmdAllocator;
	struct GfxCmdList;
	struct GfxDescriptorHeap;

	struct GfxQueueDesc
	{
		GfxQueueType type = GfxQueueType::Graphics;

		GfxQueueDesc& setType(GfxQueueType val) { type = val; return *this; }
	};

	struct GfxBufferDesc
	{
		usize size = 0;
		u32 stride = 0;
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GPU;

		GfxBufferDesc& setSize(usize val) { size = val; return *this; }
		GfxBufferDesc& setStride(u32 val) { stride = val; return *this; }
		GfxBufferDesc& setUsage(GfxBufferUsage val) { usage = val; return *this; }
		GfxBufferDesc& setMemory(GfxMemoryType val) { memory = val; return *this; }
	};

	struct GfxTextureDesc
	{
		u32 width = 1, height = 1;
		u32 depth = 1, mipLevels = 1;

		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		GfxTextureUsage usage = GfxTextureUsage::Sampled;
		GfxTextureType type = GfxTextureType::Tex2D;
	};

	struct GfxDescriptorHeapDesc
	{
		GfxDescriptorHeapType type = GfxDescriptorHeapType::Resource;
		u32 capacity = 256;
		b8 shaderVisible = false;

		GfxDescriptorHeapDesc& setType(GfxDescriptorHeapType val) { type = val; return *this; }
		GfxDescriptorHeapDesc& setCapacity(u32 val) { capacity = val; return *this; }
		GfxDescriptorHeapDesc& setShaderVisible(b8 val) { shaderVisible = val; return *this; }
	};

	struct GfxShaderBlob
	{
		const void* pData = nullptr;
		usize size = 0;
	};

	struct GfxGraphicsPipelineDesc
	{
		GfxShaderBlob taskShader, meshShader, vertexShader, computeShader, pixelShader;

		GfxTextureFormat colorFormats[8] = {};
		u32 colorTargetCount = 0;
		GfxTextureFormat depthFormat = GfxTextureFormat::Undefined;

		GfxPrimitiveTopology topology = GfxPrimitiveTopology::TriangleList;
		GfxCullMode cullMode = GfxCullMode::Back;
		GfxFrontFace frontFace = GfxFrontFace::CW;
		GfxFillMode fillMode = GfxFillMode::Solid;

		b8 depthTest = false;
		b8 depthWrite = false;
		GfxCompareOp depthCompare = GfxCompareOp::Less;

		i32 depthBias = 0;
		f32 slopeScaledDepthBias = 0.0f;

		GfxBlendState blend;

		GfxGraphicsPipelineDesc& setTS(GfxShaderBlob val) { taskShader = val; return *this; }
		GfxGraphicsPipelineDesc& setMS(GfxShaderBlob val) { meshShader = val; return *this; }
		GfxGraphicsPipelineDesc& setVS(GfxShaderBlob val) { vertexShader = val; return *this; }
		GfxGraphicsPipelineDesc& setCS(GfxShaderBlob val) { computeShader = val; return *this; }
		GfxGraphicsPipelineDesc& setPS(GfxShaderBlob val) { pixelShader = val; return *this; }
		GfxGraphicsPipelineDesc& addColorTarget(GfxTextureFormat val) { colorFormats[colorTargetCount++] = val; return *this; }
		GfxGraphicsPipelineDesc& setDepthFormat(GfxTextureFormat val) { depthFormat = val; return *this; }
		GfxGraphicsPipelineDesc& setTopology(GfxPrimitiveTopology val) { topology = val; return *this; }
		GfxGraphicsPipelineDesc& setCull(GfxCullMode val) { cullMode = val; return *this; }
		GfxGraphicsPipelineDesc& setDepth(b8 test, b8 write, GfxCompareOp op) { depthTest = test; depthWrite = write; depthCompare = op; return *this; }
		GfxGraphicsPipelineDesc& setDepthBias(i32 bias, f32 slope) { depthBias = bias; slopeScaledDepthBias = slope; return *this; }
		GfxGraphicsPipelineDesc& setBlend(const GfxBlendState& val) { blend = val; return *this; }
	};

	struct GfxComputePipelineDesc
	{
		GfxShaderBlob computeShader;

		GfxComputePipelineDesc& setCS(GfxShaderBlob val) { computeShader = val; return *this; }
	};


	struct GfxDeviceDesc
	{
		b8 enableDebug = true;
		b8 enableGPUValidation = false;

		GfxDeviceDesc& setDebug(b8 val) { enableDebug = val; return *this; }
		GfxDeviceDesc& setGPUValidation(b8 val) { enableGPUValidation = val; return *this; }
	};

	struct GfxSwapchainDesc
	{
		void* pWindowHandle = nullptr;
		u32 width = 0, height = 0;
		u32 imageCount = 2;
		b8 vSync = true;
		b8 bAllowTearing = false;

		GfxSwapchainDesc& setWindowHandle(void* pHandl) { pWindowHandle = pHandl; return *this; }
		GfxSwapchainDesc& setImgSize(u32 w, u32 h) { width = w; height = h; return *this; }
		GfxSwapchainDesc& setImgCount(u32 count) { imageCount = count; return *this; }
		GfxSwapchainDesc& setSync(b8 val) { vSync = val; return *this; }
		GfxSwapchainDesc& setAllowTear(b8 val) { bAllowTearing = val; return *this; }
	};

	struct GfxColorAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	struct GfxDepthAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};

	struct GfxRenderBeginDesc
	{
		GfxColorAttachment colorTargets[8];
		u32 colorTargetCount = 0;
		GfxDepthAttachment depth;
		u32 width = 0, height = 0;

		GfxRenderBeginDesc& addColorTarget(GfxTexture* pTex, GfxLoadOp op = GfxLoadOp::Clear,
			const glm::vec4& clear = { 0.0f, 0.0f, 0.0f, 1.0f })
		{
			colorTargets[colorTargetCount++] = { pTex, op, clear };
			return *this;
		}
		GfxRenderBeginDesc& setDepth(GfxTexture* pTex, GfxLoadOp op = GfxLoadOp::Clear, f32 clear = 1.0f)
		{
			depth = { pTex, op, clear, 0 };
			return *this;
		}
		GfxRenderBeginDesc& setSize(u32 w, u32 h) { width = w; height = h; return *this; }
	};

	struct GfxTextureBarrier
	{
		GfxTexture* pTexture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};

	namespace Gfx
	{
		GfxDevice* CreateGfxDevice(const GfxDeviceDesc& desc);
		void ShutdownGfxDevice(GfxDevice* devHandl);

		GfxQueue* CreateGfxQueue(GfxDevice* pContext, const GfxQueueDesc& desc);
		void DestroyGfxQueue(GfxQueue* qHandle);

		GfxDescriptorHeap* CreateGfxDescriptorHeap(GfxDevice* pContext, const GfxDescriptorHeapDesc& desc);
		void DestroyGfxDescriptorHeap(GfxDescriptorHeap* heapHandl);
		void FreeDescriptorIndex(GfxDescriptorHeap* pHeap, u32 index);

		GfxPipelineLayout* CreateGfxGlobalPipelineLayout(GfxDevice* pDevice);
		void DestroyGfxPipelineLayout(GfxPipelineLayout* layoutHandl);

		GfxPipeline* CreateGfxGraphicsPipeline(GfxDevice* pContext, GfxPipelineLayout* pLayout, const GfxGraphicsPipelineDesc& desc);
		GfxPipeline* CreateGfxComputePipeline(GfxDevice* pContext, GfxPipelineLayout* pLayout, const GfxComputePipelineDesc& desc);
		void DestroyGfxPipeline(GfxPipeline* plHandl);

		GfxSwapchain* CreateGfxSwapchain(GfxDevice* pContext, GfxQueue* pQueue, const GfxSwapchainDesc& desc);
		GfxTexture* RequestTexture(GfxSwapchain* pSwapchain, usize index);
		u32 GetBackbufferIndex(const GfxSwapchain* pSwapchain);
		void Present(GfxSwapchain* scHandl);
		void DestroyGfxSwapchain(GfxSwapchain* scHandl);

		GfxBuffer* CreateGfxBuffer(GfxDevice* pContext, const GfxBufferDesc& desc);
		void WriteGfxBuffer(GfxBuffer* pBuffer, const void* pData, usize sizeInBytes, usize offset);
		u32 CreateBufferSRV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxBuffer* pBuffer);
		u32 CreateBufferUAV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxBuffer* pBuffer);
		u32 GetBufferShaderView(const GfxBuffer* pBuffer);
		u32 GetBufferAccessView(const GfxBuffer* pBuffer);
		void DestroyGfxBuffer(GfxBuffer* bufHandl);

		GfxTexture* CreateGfxTexture(GfxDevice* pContext, const GfxTextureDesc& desc);
		u32 CreateTextureSRV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture);
		u32 CreateTextureRTV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture);
		u32 CreateTextureDSV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture);
		u32 CreateTextureUAV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture);
		u32 GetTextureShaderView(const GfxTexture* pTexture);
		u32 GetTextureAccessView(const GfxTexture* pTexture);
		u32 GetTextureTargetView(const GfxTexture* pTexture);
		u32 GetTextureDepthView(const GfxTexture* pTexture);
		b8 IsBackbuffer(const GfxTexture* pTexture);
		void DestroyGfxTexture(GfxTexture* texHandl);

		GfxCmdAllocator* CreateGfxCmdAllocator(GfxDevice* pDevice, GfxQueueType type);
		void ResetGfxCmdAllocator(GfxCmdAllocator* pAllocator);
		void DestroyGfxCmdAllocator(GfxCmdAllocator* pAllocator);
		GfxCmdList* CreateGfxCmdList(GfxDevice* pDevice, GfxCmdAllocator* pAllocator);
		void BeginGfxCmdList(GfxCmdList* pList, GfxCmdAllocator* pAllocator);
		void CloseGfxCmdList(GfxCmdList* pList);
		void DestroyGfxCmdList(GfxCmdList* pList);

		void CmdSetupBindless(GfxCmdList* pCmd, GfxPipelineLayout* pLayout, GfxDescriptorHeap* pResourceHeap);
		void CmdBeginRendering(GfxCmdList* pCmd, const GfxRenderBeginDesc& desc);
		void CmdBarrier(GfxCmdList* pCmd, const GfxTextureBarrier* pBarriers, u32 count);
		void CmdBindPipeline(GfxCmdList* pCmd, GfxPipeline* pPipeline);
		void CmdSetGraphicsConstants(GfxCmdList* pCmd, const void* pData, u32 count32Bit, u32 offset32Bit = 0);
		void CmdSetComputeConstants(GfxCmdList* pCmd, const void* pData, u32 count32Bit, u32 offset32Bit = 0);
		void CmdBindIndexBuffer(GfxCmdList* pCmd, GfxBuffer* pBuffer);
		void CmdDraw(GfxCmdList* pCmd, u32 vertexCount, u32 instanceCount, u32 firstVertex = 0, u32 firstInstance = 0);
		void CmdDrawIndexed(GfxCmdList* pCmd, u32 indexCount, u32 instanceCount, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0);
		void CmdDispatch(GfxCmdList* pCmd, u32 groupX, u32 groupY, u32 groupZ);
		void CmdDispatchMesh(GfxCmdList* pCmd, u32 groupX, u32 groupY, u32 groupZ);
		void CmdCopyBuffer(GfxCmdList* pCmd, GfxBuffer* pSrc, usize srcOffset, GfxBuffer* pDst, usize dstOffset, usize sizeInBytes);
		void CmdCopyBufferToTexture(GfxCmdList* pCmd, GfxBuffer* pSrc, usize srcOffset, GfxTexture* pDst, u32 mipLevel = 0, u32 arraySlice = 0);

		void ExecuteGfxCmdLists(GfxQueue* pQueue, GfxCmdList* const* ppLists, u32 count);
		u64 SignalGfxQueue(GfxQueue* pQueue);
		u64 GetGfxQueueCompleted(const GfxQueue* pQueue);
		void WaitGfxQueueCPU(GfxQueue* pQueue, u64 value);
		void WaitGfxQueueGPU(GfxQueue* pWaiter, GfxQueue* pSource, u64 value);

		void InitGfxImGui(GfxDevice* pDevice, GfxQueue* pGraphicsQueue, GfxDescriptorHeap* pResourceHeap,
			void* pWindowHandle, GfxTextureFormat targetFormat, u32 framesInFlight);
		void NewGfxImGuiFrame();
		void RenderGfxImGui(GfxCmdList* pCmd);
		void ShutdownGfxImGui();
	}
}