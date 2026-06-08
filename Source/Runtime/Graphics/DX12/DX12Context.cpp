#include "DX12Context.h"

namespace Horizon
{
	static Context dx12Context;
	Context& DX12Context() { return dx12Context; }

	u32 Helpers::AllocateDescriptor(DX12DescriptorHeap& heap)
	{
		std::lock_guard lock(heap.mutex);

		if (!heap.freeList.empty())
		{
			u32 index = heap.freeList.back();
			heap.freeList.pop_back();
			return index;
		}

		assert(heap.top < heap.capacity && "DescriptorHeap is out of space");
		return heap.top++;
	}

	void Helpers::FreeDescriptor(DX12DescriptorHeap& heap, u32 index)
	{
		std::lock_guard lock(heap.mutex);
		heap.freeList.push_back(index);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Helpers::CpuAt(const DX12DescriptorHeap& heap, u32 index)
	{
		return { heap.cpuStart.ptr + usize(index) * heap.descriptorSize };
	}

	void Helpers::CreateGlobalRootSignature()
	{
		Context& context = DX12Context();

		constexpr u32 kRootConstantCount = 16;

		D3D12_ROOT_PARAMETER1 params[1] = {};
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		params[0].Constants.ShaderRegister = 0;
		params[0].Constants.RegisterSpace = 0;
		params[0].Constants.Num32BitValues = kRootConstantCount;

		auto makeSampler = [](u32 reg, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addr,
			D3D12_COMPARISON_FUNC comp = D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR border = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
			u32 aniso = 0)
			{
				D3D12_STATIC_SAMPLER_DESC sampDesc = {};
				sampDesc.Filter = filter;
				sampDesc.AddressU = addr;
				sampDesc.AddressV = addr;
				sampDesc.AddressW = addr;
				sampDesc.MipLODBias = 0.0f;
				sampDesc.MaxAnisotropy = aniso;
				sampDesc.ComparisonFunc = comp;
				sampDesc.BorderColor = border;
				sampDesc.MinLOD = 0.0f;
				sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
				sampDesc.ShaderRegister = reg;
				sampDesc.RegisterSpace = 0;
				sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				return sampDesc;
			};

		const D3D12_STATIC_SAMPLER_DESC samplers[] =
		{
			makeSampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP),					// PointWrap
			makeSampler(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),					// PointClamp
			makeSampler(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP),					// LinearWrap
			makeSampler(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),					// LinearClamp
			makeSampler(4, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR),					// LinearMirror
			makeSampler(5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						   D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),			// AnisoWrap
			makeSampler(6, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						   D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),			// AnisoClamp
			makeSampler(7, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER),					// LinearBorder
			makeSampler(8, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						   D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),			// ShadowCmp
			makeSampler(9, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER),					// PointBorder
		};

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc = {};
		vdesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		vdesc.Desc_1_1.NumParameters = _countof(params);
		vdesc.Desc_1_1.pParameters = params;
		vdesc.Desc_1_1.NumStaticSamplers = _countof(samplers);
		vdesc.Desc_1_1.pStaticSamplers = samplers;
		vdesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* pBlob = nullptr;
		ID3DBlob* pError = nullptr;
		HRESULT bResult = D3D12SerializeVersionedRootSignature(&vdesc, &pBlob, &pError);
		CHECK_HR(bResult, "ID3D12RootSignature - D3D12SerializeVersionedRootSignature");

		bResult = context.pDevice->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
			IID_PPV_ARGS(&context.pGlobalRoot));
		CHECK_HR(bResult, "ID3D12RootSignature - CreateRootSignature");

		pBlob->Release();
	}

	void Helpers::CreateTerminalLog()
	{
		if (SUCCEEDED(context.pDevice->QueryInterface(IID_PPV_ARGS(&context.pInfoQueue))))
		{
			auto callback = [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
				D3D12_MESSAGE_ID messageId, LPCSTR desc, void* GfxContext)
				{
					std::string cString;
					std::string sString;

					switch (category)
					{
					case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:
						cString = "APPLICATION-DEFINED";
						break;
					case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:
						cString = "MISCELLANEOUS";
						break;
					case D3D12_MESSAGE_CATEGORY_INITIALIZATION:
						cString = "INITIALIZATION";
						break;
					case D3D12_MESSAGE_CATEGORY_CLEANUP:
						cString = "CLEANUP";
						break;
					case D3D12_MESSAGE_CATEGORY_COMPILATION:
						cString = "COMPILATION";
						break;
					case D3D12_MESSAGE_CATEGORY_STATE_CREATION:
						cString = "STATE-CREATION";
						break;
					case D3D12_MESSAGE_CATEGORY_STATE_SETTING:
						cString = "STATE-SETTING";
						break;
					case D3D12_MESSAGE_CATEGORY_STATE_GETTING:
						cString = "STATE-GETTING";
						break;
					case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
						cString = "RESOURCE-MANIPULATION";
						break;
					case D3D12_MESSAGE_CATEGORY_EXECUTION:
						cString = "EXECUTION";
						break;
					case D3D12_MESSAGE_CATEGORY_SHADER:
						cString = "SHADER";
						break;
					default:
						cString = "UNKNOWN";
						break;
					}

					switch (severity)
					{
					case D3D12_MESSAGE_SEVERITY_CORRUPTION:
						sString = "CORRUPTION";
						break;
					case D3D12_MESSAGE_SEVERITY_ERROR:
						sString = "ERROR";
						break;
					case D3D12_MESSAGE_SEVERITY_WARNING:
						sString = "WARNING";
						break;
					case D3D12_MESSAGE_SEVERITY_INFO:
						sString = "INFO";
						break;
					case D3D12_MESSAGE_SEVERITY_MESSAGE:
						sString = "MESSAGE";
						break;
					}

					MainLog::Terminal("[{}][{}]: {}", cString, sString, desc);
				};

			context.pInfoQueue->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &context.pInfoId);
		}
	}

	void Helpers::CreateDescriptorHeap(DX12DescriptorHeap& heap, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, b8 shaderVisible)
	{
		Context& context = DX12Context();

		assert((!shaderVisible || type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) &&
			"Only CBV_SRV_UAV / SAMPLER heaps can be shader-visible");

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = capacity;
		desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		HRESULT bResult = context.pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap.pHeap));
		CHECK_HR(bResult, "ID3D12DescriptorHeap - CreateDescriptorHeap");

		heap.descriptorSize = context.pDevice->GetDescriptorHandleIncrementSize(type);
		heap.capacity = capacity;
		heap.cpuStart = heap.pHeap->GetCPUDescriptorHandleForHeapStart();
		heap.gpuStart = shaderVisible ? heap.pHeap->GetGPUDescriptorHandleForHeapStart()
			: D3D12_GPU_DESCRIPTOR_HANDLE{};

		heap.top = 0;
		heap.freeList.clear();
	}

	u32 Helpers::CreateBufferSRV(DX12Buffer& b, const GfxBufferDesc& desc)
	{

	}
	u32 Helpers::CreateBufferUAV(DX12Buffer& b, const GfxBufferDesc& desc)
	{

	}

}