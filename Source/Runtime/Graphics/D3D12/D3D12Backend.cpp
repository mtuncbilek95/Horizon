#include "D3D12Backend.h"

namespace Horizon
{
	void Helpers::CreateTerminalLog(GfxDevice* pContext)
	{
		if (SUCCEEDED(pContext->pDevice->QueryInterface(IID_PPV_ARGS(&pContext->pInfoQueue))))
		{
			auto callback = [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
				D3D12_MESSAGE_ID messageId, LPCSTR desc, void* ctx)
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

			pContext->pInfoQueue->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &pContext->pInfoId);
		}
	}

	u32 Helpers::AllocateDescriptor(GfxDescriptorHeap* pHeap)
	{
		if (!pHeap->freeList.empty())
		{
			u32 index = pHeap->freeList.back();
			pHeap->freeList.pop_back();
			return index;
		}

		assert(pHeap->top < pHeap->capacity && "DescriptorHeap is out of space");
		return pHeap->top++;
	}

	void Helpers::FreeDescriptor(GfxDescriptorHeap* pHeap, u32 index)
	{
		assert(index < pHeap->top && "Descriptor index was never allocated from this heap");
		pHeap->freeList.push_back(index);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Helpers::CpuAt(const GfxDescriptorHeap* pHeap, u32 index)
	{
		return { pHeap->cpuStart.ptr + usize(index) * pHeap->descriptorSize };
	}

	D3D12_GPU_DESCRIPTOR_HANDLE Helpers::GpuAt(const GfxDescriptorHeap* pHeap, u32 index)
	{
		return { pHeap->gpuStart.ptr + u64(index) * pHeap->descriptorSize };
	}

	DXGI_FORMAT Helpers::ToDXGIFormat(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::R8:
			return DXGI_FORMAT_R8_UNORM;
		case GfxTextureFormat::RG8:
			return DXGI_FORMAT_R8G8_UNORM;
		case GfxTextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GfxTextureFormat::RGBA8_sRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case GfxTextureFormat::BGRA8:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GfxTextureFormat::BGRA8_sRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case GfxTextureFormat::R16F:
			return DXGI_FORMAT_R16_FLOAT;
		case GfxTextureFormat::RG16F:
			return DXGI_FORMAT_R16G16_FLOAT;
		case GfxTextureFormat::RGBA16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case GfxTextureFormat::R32F:
			return DXGI_FORMAT_R32_FLOAT;
		case GfxTextureFormat::RG32F:
			return DXGI_FORMAT_R32G32_FLOAT;
		case GfxTextureFormat::RGBA32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case GfxTextureFormat::R11G11B10F:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case GfxTextureFormat::RGB10A2:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case GfxTextureFormat::R32U:
			return DXGI_FORMAT_R32_UINT;
		case GfxTextureFormat::RG32U:
			return DXGI_FORMAT_R32G32_UINT;
		case GfxTextureFormat::D16:
			return DXGI_FORMAT_D16_UNORM;
		case GfxTextureFormat::D32:
			return DXGI_FORMAT_D32_FLOAT;
		case GfxTextureFormat::D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GfxTextureFormat::BC7:
			return DXGI_FORMAT_BC7_UNORM;
		case GfxTextureFormat::BC7_sRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT Helpers::ToTypelessFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:
			return DXGI_FORMAT_R16_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;
		default:
			return format;
		}
	}

	DXGI_FORMAT Helpers::ToDepthSRVFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default:
			return format;
		}
	}

	b8 Helpers::IsDepthFormat(GfxTextureFormat format)
	{
		return format == GfxTextureFormat::D16 || format == GfxTextureFormat::D32
			|| format == GfxTextureFormat::D24S8;
	}

	D3D12_BLEND Helpers::ToBlend(GfxBlendFactor factor)
	{
		switch (factor)
		{
		case GfxBlendFactor::Zero:
			return D3D12_BLEND_ZERO;
		case GfxBlendFactor::One:
			return D3D12_BLEND_ONE;
		case GfxBlendFactor::SrcColor:
			return D3D12_BLEND_SRC_COLOR;
		case GfxBlendFactor::InvSrcColor:
			return D3D12_BLEND_INV_SRC_COLOR;
		case GfxBlendFactor::SrcAlpha:
			return D3D12_BLEND_SRC_ALPHA;
		case GfxBlendFactor::InvSrcAlpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case GfxBlendFactor::DstColor:
			return D3D12_BLEND_DEST_COLOR;
		case GfxBlendFactor::InvDstColor:
			return D3D12_BLEND_INV_DEST_COLOR;
		case GfxBlendFactor::DstAlpha:
			return D3D12_BLEND_DEST_ALPHA;
		case GfxBlendFactor::InvDstAlpha:
			return D3D12_BLEND_INV_DEST_ALPHA;
		default:
			return D3D12_BLEND_ONE;
		}
	}

	D3D12_BLEND_OP Helpers::ToBlendOp(GfxBlendOp op)
	{
		switch (op)
		{
		case GfxBlendOp::Add:
			return D3D12_BLEND_OP_ADD;
		case GfxBlendOp::Subtract:
			return D3D12_BLEND_OP_SUBTRACT;
		case GfxBlendOp::RevSubtract:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case GfxBlendOp::Min:
			return D3D12_BLEND_OP_MIN;
		case GfxBlendOp::Max:
			return D3D12_BLEND_OP_MAX;
		default:
			return D3D12_BLEND_OP_ADD;
		}
	}

	D3D12_COMPARISON_FUNC Helpers::ToCompare(GfxCompareOp op)
	{
		switch (op)
		{
		case GfxCompareOp::Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case GfxCompareOp::Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case GfxCompareOp::Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case GfxCompareOp::LessEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case GfxCompareOp::Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case GfxCompareOp::NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GfxCompareOp::GreaterEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case GfxCompareOp::Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		default:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE Helpers::ToTopologyType(GfxPrimitiveTopology topology)
	{
		switch (topology)
		{
		case GfxPrimitiveTopology::PointList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case GfxPrimitiveTopology::LineList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	D3D_PRIMITIVE_TOPOLOGY Helpers::ToTopology(GfxPrimitiveTopology topology)
	{
		switch (topology)
		{
		case GfxPrimitiveTopology::PointList:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case GfxPrimitiveTopology::LineList:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case GfxPrimitiveTopology::TriangleStrip:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	D3D12_RESOURCE_STATES Helpers::ToResourceState(GfxResourceState state)
	{
		switch (state)
		{
		case GfxResourceState::Common:
			return D3D12_RESOURCE_STATE_COMMON;
		case GfxResourceState::VertexBuffer:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case GfxResourceState::IndexBuffer:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case GfxResourceState::ConstantBuffer:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case GfxResourceState::IndirectArg:
			return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		case GfxResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case GfxResourceState::UnorderedAccess:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case GfxResourceState::RenderTarget:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case GfxResourceState::DepthWrite:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case GfxResourceState::DepthRead:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case GfxResourceState::CopySrc:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case GfxResourceState::CopyDst:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case GfxResourceState::Present:
			return D3D12_RESOURCE_STATE_PRESENT;
		default:
			return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	D3D12_COMMAND_LIST_TYPE Helpers::ToListType(GfxQueueType type)
	{
		switch (type)
		{
		case GfxQueueType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case GfxQueueType::Transfer:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		default:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}
}