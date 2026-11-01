#include "D3D12Device.h"

#include <Runtime/D3D12/Queue/D3D12Queue.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	namespace
	{
		std::string ResultToInfo(HRESULT result)
		{

		}
	}

	RUNTIME_API UniquePtr<Device> CreateRHIDevice(DeviceDesc& desc)
	{
		UniquePtr<D3D12Device> pDevice = MakeUnique<D3D12Device>();
		if (pDevice->Initialize(desc))
			return pDevice;

		return nullptr;
	}

	D3D12Device::D3D12Device()
	{
	}

	D3D12Device::~D3D12Device()
	{
		CloseAll();
	}

	b8 D3D12Device::Initialize(const DeviceDesc& desc)
	{
		// Handle debug
		u32 factoryFlags = 0;
		if (desc.enableDebug && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
		{
			m_debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			if (desc.enableGpuValidation)
				m_debug->SetEnableGPUBasedValidation(TRUE);

			if (desc.synchronizedCommandValidation)
				m_debug->SetEnableGPUBasedValidation(TRUE);
		}

		// Create the instance
		HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateDXGIFactory2:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Get the best adapter
		// TODO: Adapter generation can be a thing in future
		hr = m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&m_adapter));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "EnumAdapterByGpuPreference:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Create the base device
		ID3D12Device* pBaseDevice = nullptr;
		hr = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pBaseDevice));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "D3D12CreateDevice:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Handle the query interface
		// Self-note: Why the fuck its named as QueryInterface? Isn't it just a fancy casting?
		hr = pBaseDevice->QueryInterface(IID_PPV_ARGS(&m_device));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "QueryInterface To ID3DDevice10:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}
		pBaseDevice->Release();

		// Create the allocator
		D3D12MA::ALLOCATOR_DESC allocDesc = {};

		allocDesc.pDevice = m_device;
		allocDesc.pAdapter = m_adapter;
		hr = D3D12MA::CreateAllocator(&allocDesc, &m_allocator);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateAllocator:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Pop up the errors and warnings on terminal
		RegisterTerminalMessage();
	}

	void D3D12Device::CloseAll()
	{
		if (m_allocator)
			m_allocator->Release();

		if (m_infoQueue)
		{
			m_infoQueue->UnregisterMessageCallback(m_callbackCookie);
			m_infoQueue->Release();
		}

		if (m_device)
			m_device->Release();

		if (m_adapter)
			m_adapter->Release();

		if (m_factory)
			m_factory->Release();

		if (m_debug)
			m_debug->Release();
	}

	UniquePtr<Queue> D3D12Device::CreateRHIQueue(const QueueDesc& desc)
	{
		UniquePtr<D3D12Queue> pNewQueue = MakeUnique<D3D12Queue>();

		pNewQueue->m_ownerDevice = this;
		pNewQueue->m_type = desc.type;

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

		if (desc.type == QueueType::Graphics)
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		else if (desc.type == QueueType::Compute)
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		else if (desc.type == QueueType::Transfer)
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		else
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;




		return nullptr;
	}

	UniquePtr<Buffer> D3D12Device::CreateRHIBuffer(const BufferDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<Texture> D3D12Device::CreateRHITexture(const TextureDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<TextureView> D3D12Device::CreateRHITextureView(const TextureViewDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<Sampler> D3D12Device::CreateRHISampler(const SamplerDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<ShaderModule> D3D12Device::CreateRHIShaderModule(const ShaderModuleDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<PipelineLayout> Device::CreateRHIGlobalPipelineLayout()
	{
		return nullptr;
	}

	UniquePtr<PipelineLayout> D3D12Device::CreateRHIPipelineLayout(const PipelineLayoutDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<GraphicsPipeline> D3D12Device::CreateRHIPipeline(const GraphicsPipelineDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<ComputePipeline> D3D12Device::CreateRHIPipeline(const ComputePipelineDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<DescriptorHeap> D3D12Device::CreateRHIDescriptorHeap(const DescriptorHeapDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<DescriptorTable> D3D12Device::CreateRHIDescriptorTable(const DescriptorTableDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<CommandPool> D3D12Device::CreateRHICommandPool(const CommandPoolDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<CommandList> D3D12Device::CreateRHICommandList(const CommandListDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<Fence> D3D12Device::CreateRHIFence(const FenceDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<QueryPool> D3D12Device::CreateRHIQueryPool(const QueryPoolDesc& desc)
	{
		return nullptr;
	}

	UniquePtr<Swapchain> D3D12Device::CreateRHISwapchain(const SwapchainDesc& desc)
	{
		return nullptr;
	}

	void D3D12Device::RegisterTerminalMessage()
	{
		if (FAILED(m_device->QueryInterface(IID_PPV_ARGS(&m_infoQueue))))
			return;

		auto cb = [](D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY sev,
			D3D12_MESSAGE_ID, LPCSTR pDesc, void*)
			{
				switch (sev)
				{
				case D3D12_MESSAGE_SEVERITY_CORRUPTION:
					Terminal::Error("D3D12", "CORRUPTION: {}", pDesc);
					break;
				case D3D12_MESSAGE_SEVERITY_ERROR:
					Terminal::Error("D3D12", "ERROR: {}", pDesc);
					break;
				case D3D12_MESSAGE_SEVERITY_WARNING:
					Terminal::Warn("D3D12", "WARNING: {}", pDesc);
					break;
				case D3D12_MESSAGE_SEVERITY_INFO:
					Terminal::Debug("D3D12", "INFO: {}", pDesc);
					break;
				default:
					Terminal::Debug("D3D12", "MESSAGE: {}", pDesc);
					break;
				}
			};

		m_infoQueue->RegisterMessageCallback(cb, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_callbackCookie);
	}

}