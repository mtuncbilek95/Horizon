#include "DX12Pipeline.h"

#include <Runtime/Graphics/RHI/GfxPipeline.h>
#include <Runtime/Graphics/RHI/GfxDevice.h>

#include <Runtime/Graphics/DX12/DX12Device.h>
#include <Runtime/Graphics/DX12/DX12Utils.h>

#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { Horizon::MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

namespace Horizon
{
	static D3D12_FILL_MODE ToFill(GfxFillMode m)
	{
		return m == GfxFillMode::Wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	}

	static D3D12_CULL_MODE ToCull(GfxCullMode m)
	{
		switch (m)
		{
		case GfxCullMode::Front: return D3D12_CULL_MODE_FRONT;
		case GfxCullMode::Back:  return D3D12_CULL_MODE_BACK;
		default:                 return D3D12_CULL_MODE_NONE;
		}
	}

	static D3D12_COMPARISON_FUNC ToCompare(GfxCompareOp op)
	{
		switch (op)
		{
		case GfxCompareOp::Never:        return D3D12_COMPARISON_FUNC_NEVER;
		case GfxCompareOp::Less:         return D3D12_COMPARISON_FUNC_LESS;
		case GfxCompareOp::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
		case GfxCompareOp::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case GfxCompareOp::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
		case GfxCompareOp::NotEqual:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GfxCompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		default:                         return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	static D3D12_BLEND ToBlend(GfxBlendFactor f)
	{
		switch (f)
		{
		case GfxBlendFactor::Zero:       return D3D12_BLEND_ZERO;
		case GfxBlendFactor::One:        return D3D12_BLEND_ONE;
		case GfxBlendFactor::SrcColor:   return D3D12_BLEND_SRC_COLOR;
		case GfxBlendFactor::InvSrcColor:return D3D12_BLEND_INV_SRC_COLOR;
		case GfxBlendFactor::SrcAlpha:   return D3D12_BLEND_SRC_ALPHA;
		case GfxBlendFactor::InvSrcAlpha:return D3D12_BLEND_INV_SRC_ALPHA;
		case GfxBlendFactor::DstColor:   return D3D12_BLEND_DEST_COLOR;
		case GfxBlendFactor::InvDstColor:return D3D12_BLEND_INV_DEST_COLOR;
		case GfxBlendFactor::DstAlpha:   return D3D12_BLEND_DEST_ALPHA;
		case GfxBlendFactor::InvDstAlpha:return D3D12_BLEND_INV_DEST_ALPHA;
		default:                         return D3D12_BLEND_ONE;
		}
	}

	static D3D12_BLEND_OP ToBlendOp(GfxBlendOp op)
	{
		switch (op)
		{
		case GfxBlendOp::Subtract:    return D3D12_BLEND_OP_SUBTRACT;
		case GfxBlendOp::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
		case GfxBlendOp::Min:         return D3D12_BLEND_OP_MIN;
		case GfxBlendOp::Max:         return D3D12_BLEND_OP_MAX;
		default:                      return D3D12_BLEND_OP_ADD;
		}
	}

	static void ToTopology(GfxPrimitiveTopology t, D3D12_PRIMITIVE_TOPOLOGY_TYPE& outType, D3D_PRIMITIVE_TOPOLOGY& outTopo)
	{
		switch (t)
		{
		case GfxPrimitiveTopology::PointList:
			outType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; outTopo = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; break;
		case GfxPrimitiveTopology::LineList:
			outType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; outTopo = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
		case GfxPrimitiveTopology::TriangleStrip:
			outType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; outTopo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
		default:
			outType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; outTopo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		}
	}

	static D3D12_BLEND_DESC MakeBlend(const GfxGraphicsPipelineDesc& d)
	{
		D3D12_RENDER_TARGET_BLEND_DESC rt = {};
		rt.BlendEnable = d.blendEnable;
		rt.SrcBlend = ToBlend(d.srcColor);
		rt.DestBlend = ToBlend(d.dstColor);
		rt.BlendOp = ToBlendOp(d.colorOp);
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_ZERO;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_BLEND_DESC blend = {};
		for (u32 i = 0; i < 8; ++i)
			blend.RenderTarget[i] = rt;
		return blend;
	}

	static D3D12_RASTERIZER_DESC MakeRaster(const GfxGraphicsPipelineDesc& d)
	{
		D3D12_RASTERIZER_DESC r = {};
		r.FillMode = ToFill(d.fill);
		r.CullMode = ToCull(d.cull);
		r.FrontCounterClockwise = (d.frontFace == GfxFrontFace::CCW);
		r.DepthClipEnable = TRUE;
		return r;
	}

	static D3D12_DEPTH_STENCIL_DESC MakeDepth(const GfxGraphicsPipelineDesc& d)
	{
		D3D12_DEPTH_STENCIL_DESC ds = {};
		ds.DepthEnable = d.depthTest;
		ds.DepthWriteMask = d.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		ds.DepthFunc = ToCompare(d.depthCompare);
		ds.StencilEnable = FALSE;
		return ds;
	}

	static D3D12_RT_FORMAT_ARRAY MakeRTFormats(const GfxGraphicsPipelineDesc& d)
	{
		D3D12_RT_FORMAT_ARRAY a = {};
		a.NumRenderTargets = d.colorCount;
		for (u32 i = 0; i < d.colorCount; ++i)
			a.RTFormats[i] = ToDXGI(d.colorFormats[i]);
		return a;
	}

	template<typename Inner, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type>
	struct alignas(void*) PsoSub
	{
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = Type;
		Inner inner{};
	};

	struct VertexStream
	{
		PsoSub<ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE> rootSig;
		PsoSub<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS> vs;
		PsoSub<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS> gs;
		PsoSub<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS> ps;
		PsoSub<D3D12_BLEND_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND> blend;
		PsoSub<D3D12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER> raster;
		PsoSub<D3D12_DEPTH_STENCIL_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL> depth;
		PsoSub<D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY> topo;
		PsoSub<D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS> rtFormats;
		PsoSub<DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT> dsvFormat;
		PsoSub<DXGI_SAMPLE_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC> sampleDesc;
	};

	struct MeshStream
	{
		PsoSub<ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE> rootSig;
		PsoSub<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS> as;
		PsoSub<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS> ms;
		PsoSub<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS> ps;
		PsoSub<D3D12_BLEND_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND> blend;
		PsoSub<D3D12_RASTERIZER_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER> raster;
		PsoSub<D3D12_DEPTH_STENCIL_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL> depth;
		PsoSub<D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS> rtFormats;
		PsoSub<DXGI_FORMAT, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT> dsvFormat;
		PsoSub<DXGI_SAMPLE_DESC, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC> sampleDesc;
	};

	static D3D12_SHADER_BYTECODE ToBytecode(const GfxShaderBytecode& b)
	{
		return D3D12_SHADER_BYTECODE{ b.data, b.size };
	}

	GfxPipeline::GfxPipeline(const GfxGraphicsPipelineDesc& desc, GfxDevice* pDevice)
		: GfxObject(pDevice), m_type(GfxPipelineType::Graphics)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());
		DX12Pipeline* pNative = new DX12Pipeline();
		pNative->isCompute = false;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType;
		ToTopology(desc.topology, topoType, pNative->topology);

		const DXGI_FORMAT dsvFormat = (desc.depthFormat == GfxTextureFormat::Undefined)
			? DXGI_FORMAT_UNKNOWN : ToDXGI(desc.depthFormat);

		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};

		if (desc.mesh.data != nullptr)
		{
			MeshStream stream = {};
			stream.rootSig.inner = deviceNative->globalRootSignature;
			stream.as.inner = ToBytecode(desc.task);
			stream.ms.inner = ToBytecode(desc.mesh);
			stream.ps.inner = ToBytecode(desc.pixel);
			stream.blend.inner = MakeBlend(desc);
			stream.raster.inner = MakeRaster(desc);
			stream.depth.inner = MakeDepth(desc);
			stream.rtFormats.inner = MakeRTFormats(desc);
			stream.dsvFormat.inner = dsvFormat;
			stream.sampleDesc.inner = { 1, 0 };

			streamDesc.SizeInBytes = sizeof(stream);
			streamDesc.pPipelineStateSubobjectStream = &stream;
			CHECK_HR(deviceNative->device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pNative->pso)), "CreatePipelineState (mesh)");
		}
		else
		{
			VertexStream stream = {};
			stream.rootSig.inner = deviceNative->globalRootSignature;
			stream.vs.inner = ToBytecode(desc.vertex);
			stream.gs.inner = ToBytecode(desc.geom);
			stream.ps.inner = ToBytecode(desc.pixel);
			stream.blend.inner = MakeBlend(desc);
			stream.raster.inner = MakeRaster(desc);
			stream.depth.inner = MakeDepth(desc);
			stream.topo.inner = topoType;
			stream.rtFormats.inner = MakeRTFormats(desc);
			stream.dsvFormat.inner = dsvFormat;
			stream.sampleDesc.inner = { 1, 0 };

			streamDesc.SizeInBytes = sizeof(stream);
			streamDesc.pPipelineStateSubobjectStream = &stream;
			CHECK_HR(deviceNative->device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pNative->pso)), "CreatePipelineState (vertex)");
		}

		m_native = pNative;
	}

	GfxPipeline::GfxPipeline(const GfxComputePipelineDesc& desc, GfxDevice* pDevice)
		: GfxObject(pDevice), m_type(GfxPipelineType::Compute)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());
		DX12Pipeline* pNative = new DX12Pipeline();
		pNative->isCompute = true;

		D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
		computeDesc.pRootSignature = deviceNative->globalRootSignature;
		computeDesc.CS = ToBytecode(desc.compute);

		CHECK_HR(deviceNative->device->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&pNative->pso)), "CreateComputePipelineState");

		m_native = pNative;
	}

	GfxPipeline::~GfxPipeline()
	{
		DX12Pipeline* pNative = static_cast<DX12Pipeline*>(m_native);

		m_device->DeferDestroy([pNative]
			{
				if (pNative->pso)
					pNative->pso->Release();
				delete pNative;
			});
	}
}
