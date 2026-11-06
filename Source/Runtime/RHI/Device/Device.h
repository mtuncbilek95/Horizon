#pragma once

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/ReadOnlyList.h>
#include <Runtime/RHI/Device/DeviceDesc.h>
#include <Runtime/RHI/Queue/QueueType.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::RHI
{
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

	struct SwapchainDesc;
	class Swapchain;

	class Device
	{
	public:
		virtual ~Device() = default;

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
		virtual UniquePtr<Swapchain> CreateRHISwapchain(const SwapchainDesc& desc) = 0;

		virtual void Submit(QueueType type, ReadOnlyList<CommandList*> lists) = 0;
		virtual u64 Signal(QueueType type, Fence* pFence) = 0;
		virtual void Wait(QueueType type, Fence* pFence, u64 value) = 0;
		virtual void WaitIdle() = 0;

		virtual u64 GetTimestampFrequency(QueueType type) = 0;

		virtual void InitializeImGui(u32 framesInFlight, DescriptorHeap* pResourceHeap, TextureFormat colorFormat) = 0;
		virtual void NewFrameImGui() = 0;
		virtual void ShutdownImGui() = 0;
	};

	UniquePtr<Device> CreateRHIDevice(DeviceDesc& desc);
}