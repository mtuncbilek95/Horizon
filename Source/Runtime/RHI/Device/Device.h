#pragma once

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RHI/Device/DeviceDesc.h>

namespace Horizon::RHI
{
	struct QueueDesc;
	class Queue;

	struct BufferDesc;
	class Buffer;

	struct TextureDesc;
	class Texture;

	struct TextureViewDesc;
	class TextureView;

	struct SamplerDesc;
	class Sampler;

	struct ShaderModuleDesc;
	class ShaderModule;

	struct PipelineLayoutDesc;
	class PipelineLayout;

	struct GraphicsPipelineDesc;
	class GraphicsPipeline;

	struct ComputePipelineDesc;
	class ComputePipeline;

	struct DescriptorHeapDesc;
	class DescriptorHeap;

	struct DescriptorTableDesc;
	class DescriptorTable;

	struct CommandPoolDesc;
	class CommandPool;

	struct CommandListDesc;
	class CommandList;

	struct FenceDesc;
	class Fence;
	
	struct QueryPoolDesc;
	class QueryPool;
	
	struct SwapchainDesc;
	class Swapchain;

	class RUNTIME_API Device
	{
	public:
		virtual ~Device() = default;

		virtual UniquePtr<Queue> CreateRHIQueue(const QueueDesc& desc) = 0;
		virtual UniquePtr<Buffer> CreateRHIBuffer(const BufferDesc& desc) = 0;
		virtual UniquePtr<Texture> CreateRHITexture(const TextureDesc& desc) = 0;
		virtual UniquePtr<TextureView> CreateRHITextureView(const TextureViewDesc& desc) = 0;
		virtual UniquePtr<Sampler> CreateRHISampler(const SamplerDesc& desc) = 0;
		virtual UniquePtr<ShaderModule> CreateRHIShaderModule(const ShaderModuleDesc& desc) = 0;
		virtual UniquePtr<PipelineLayout> CreateRHIGlobalPipelineLayout();
		virtual UniquePtr<PipelineLayout> CreateRHIPipelineLayout(const PipelineLayoutDesc& desc) = 0;
		virtual UniquePtr<GraphicsPipeline> CreateRHIPipeline(const GraphicsPipelineDesc& desc) = 0;
		virtual UniquePtr<ComputePipeline> CreateRHIPipeline(const ComputePipelineDesc& desc) = 0;
		virtual UniquePtr<DescriptorHeap> CreateRHIDescriptorHeap(const DescriptorHeapDesc& desc) = 0;
		virtual UniquePtr<DescriptorTable> CreateRHIDescriptorTable(const DescriptorTableDesc& desc) = 0;
		virtual UniquePtr<CommandPool> CreateRHICommandPool(const CommandPoolDesc& desc) = 0;
		virtual UniquePtr<CommandList> CreateRHICommandList(const CommandListDesc& desc) = 0;
		virtual UniquePtr<Fence> CreateRHIFence(const FenceDesc& desc) = 0;
		virtual UniquePtr<QueryPool> CreateRHIQueryPool(const QueryPoolDesc& desc) = 0;
		virtual UniquePtr<Swapchain> CreateRHISwapchain(const SwapchainDesc& desc) = 0;
	};

	RUNTIME_API UniquePtr<Device> CreateRHIDevice(DeviceDesc& desc);
}