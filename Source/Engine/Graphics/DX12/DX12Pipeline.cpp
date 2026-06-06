#include "DX12Backend.h"

#include <Engine/Log/Log.h>
#include <Engine/Graphics/RHI/GfxPipeline.h>

#define CHECK_HR(hr, what) \
	if (FAILED(hr)) { ConsoleLog().Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

namespace Horizon::DX12
{
	static constexpr u8 MaxSamplers = 6;

	GfxPipelineHandle PipelinePoolAlloc()
	{
		Context& context = GfxContext();
		u32 index;
		if (!context.pipeFreeList.empty())
		{
			index = context.pipeFreeList.back();
			context.pipeFreeList.pop_back();
		}
		else
		{
			index = (u32)context.pipelines.size();
			context.pipelines.push_back({});
			context.pipeGenerations.push_back(0);
		}

		return GfxPipelineHandle::make(index, context.pipeGenerations[index]);
	}

	DX12Pipeline& PipelinePoolGet(GfxPipelineHandle handle) { return GfxContext().pipelines[handle.index()]; }

	void PipelinePoolFree(GfxPipelineHandle handle)
	{
		Context& context = GfxContext();
		context.pipelines[handle.index()] = {};
		context.pipeGenerations[handle.index()]++;
		context.pipeFreeList.push_back(handle.index());
	}

	static D3D12_CULL_MODE ToCull(GfxCullMode cull) { return cull == GfxCullMode::None ? D3D12_CULL_MODE_NONE : cull == GfxCullMode::Front ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK; }
	static D3D12_COMPARISON_FUNC ToCompare(GfxCompareOp op) { return D3D12_COMPARISON_FUNC(u32(op) + 1); }
	static D3D12_BLEND_OP ToBlendOp(GfxBlendOp op) { return D3D12_BLEND_OP(u32(op) + 1); }

	static D3D12_BLEND ToBlend(GfxBlendFactor factor)
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

	static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopoType(GfxPrimitiveTopology topology)
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

	static D3D_PRIMITIVE_TOPOLOGY ToTopo(GfxPrimitiveTopology topology)
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

	void CreateGlobalRootSignature()
	{
		Context& context = GfxContext();

		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		param.Constants.ShaderRegister = 0;
		param.Constants.Num32BitValues = 32;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		auto sampler = [](u32 shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE address,
			D3D12_COMPARISON_FUNC compareFunc = D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR border = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK)
			{
				D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
				samplerDesc.Filter = filter;
				samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = address;
				samplerDesc.MaxAnisotropy = 8;
				samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
				samplerDesc.ComparisonFunc = compareFunc;
				samplerDesc.BorderColor = border;
				samplerDesc.ShaderRegister = shaderRegister;
				samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				return samplerDesc;
			};

		D3D12_STATIC_SAMPLER_DESC samplers[MaxSamplers] =
		{
			sampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			sampler(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			sampler(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			sampler(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			sampler(4, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			sampler(5, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
				D3D12_COMPARISON_FUNC_GREATER_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),
		};

		D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		rootSigDesc.NumParameters = 1;
		rootSigDesc.pParameters = &param;
		rootSigDesc.NumStaticSamplers = MaxSamplers;
		rootSigDesc.pStaticSamplers = samplers;
		rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* sigBlob = nullptr; ID3DBlob* errorBlob = nullptr;
		D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
		context.device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&context.rootSignature));

		if (sigBlob)
			sigBlob->Release();
		if (errorBlob)
			errorBlob->Release();
	}
}

namespace Horizon
{
	using namespace DX12;

	GfxPipelineHandle GfxDevice::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		Context& context = GfxContext();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
		pipelineDesc.pRootSignature = context.rootSignature;
		pipelineDesc.VS = { desc.vertex.data, desc.vertex.size };
		pipelineDesc.PS = { desc.pixel.data, desc.pixel.size };

		pipelineDesc.InputLayout = { nullptr, 0 };

		pipelineDesc.RasterizerState.FillMode = (desc.fill == GfxFillMode::Wireframe) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		pipelineDesc.RasterizerState.CullMode = ToCull(desc.cull);
		pipelineDesc.RasterizerState.FrontCounterClockwise = (desc.frontFace == GfxFrontFace::CCW);
		pipelineDesc.RasterizerState.DepthClipEnable = TRUE;

		pipelineDesc.BlendState.RenderTarget[0].BlendEnable = desc.blendEnable;
		pipelineDesc.BlendState.RenderTarget[0].SrcBlend = ToBlend(desc.srcColor);
		pipelineDesc.BlendState.RenderTarget[0].DestBlend = ToBlend(desc.dstColor);
		pipelineDesc.BlendState.RenderTarget[0].BlendOp = ToBlendOp(desc.colorOp);
		pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		pipelineDesc.DepthStencilState.DepthEnable = desc.depthTest;
		pipelineDesc.DepthStencilState.DepthWriteMask = desc.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		pipelineDesc.DepthStencilState.DepthFunc = ToCompare(desc.depthCompare);

		pipelineDesc.PrimitiveTopologyType = ToTopoType(desc.topology);
		pipelineDesc.SampleMask = 0xFFFFFFFF;
		pipelineDesc.SampleDesc.Count = 1;

		pipelineDesc.NumRenderTargets = desc.colorCount;
		for (u32 colorIdx = 0; colorIdx < desc.colorCount; colorIdx++)
			pipelineDesc.RTVFormats[colorIdx] = ToDXGI(desc.colorFormats[colorIdx]);

		pipelineDesc.DSVFormat = (desc.depthFormat == GfxTextureFormat::Undefined) ? DXGI_FORMAT_UNKNOWN : ToDXGI(desc.depthFormat);

		GfxPipelineHandle handle = PipelinePoolAlloc();
		DX12Pipeline& pipeline = PipelinePoolGet(handle);
		pipeline.isCompute = false;
		pipeline.topology = ToTopo(desc.topology);
		HRESULT hresult = context.device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipeline.pso));
		CHECK_HR(hresult, "Create Graphics Pipeline State");

		SetDebugName(pipeline.pso, desc.debugName);

		return handle;
	}

	GfxPipelineHandle GfxDevice::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		Context& context = GfxContext();

		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc = {};
		pipelineDesc.pRootSignature = context.rootSignature;
		pipelineDesc.CS = { desc.compute.data, desc.compute.size };

		GfxPipelineHandle handle = PipelinePoolAlloc();
		DX12Pipeline& pipeline = PipelinePoolGet(handle);
		pipeline.isCompute = true;
		context.device->CreateComputePipelineState(&pipelineDesc, IID_PPV_ARGS(&pipeline.pso));

		return handle;
	}

	void GfxDevice::DestroyPipeline(GfxPipelineHandle handle)
	{
		DX12Pipeline& pipeline = PipelinePoolGet(handle);
		if (pipeline.pso)
			pipeline.pso->Release();
		PipelinePoolFree(handle);
	}
}
