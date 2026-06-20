#include "D3D12Pipeline.h"

#include <Runtime/D3D12/D3D12Device.h>

namespace Horizon
{
	namespace
	{
		template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type, typename T>
		struct alignas(void*) StreamSubobject
		{
			D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = Type;
			T value = {};
		};

		struct MeshStateStream
		{
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*> rootSignature;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE> taskShader;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE> meshShader;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE> pixelShader;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC> blend;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, UINT> sampleMask;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC> rasterizer;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, D3D12_DEPTH_STENCIL_DESC> depthStencil;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE> topology;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY> rtFormats;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT> dsFormat;
			StreamSubobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC> sampleDesc;
		};

		D3D12_BLEND_DESC BuildBlendDesc(const GfxGraphicsPipelineDesc& desc)
		{
			D3D12_RENDER_TARGET_BLEND_DESC target = {};
			target.BlendEnable = desc.blend.enable;
			target.SrcBlend = Helpers::ToBlend(desc.blend.srcColor);
			target.DestBlend = Helpers::ToBlend(desc.blend.dstColor);
			target.BlendOp = Helpers::ToBlendOp(desc.blend.colorOp);
			target.SrcBlendAlpha = Helpers::ToBlend(desc.blend.srcAlpha);
			target.DestBlendAlpha = Helpers::ToBlend(desc.blend.dstAlpha);
			target.BlendOpAlpha = Helpers::ToBlendOp(desc.blend.alphaOp);
			target.RenderTargetWriteMask = u8(desc.blend.writeMask);

			D3D12_BLEND_DESC blendDesc = {};
			for (u32 i = 0; i < desc.colorTargetCount; i++)
				blendDesc.RenderTarget[i] = target;

			return blendDesc;
		}

		D3D12_RASTERIZER_DESC BuildRasterizerDesc(const GfxGraphicsPipelineDesc& desc)
		{
			D3D12_RASTERIZER_DESC rasterDesc = {};
			rasterDesc.FillMode = desc.fillMode == GfxFillMode::Wireframe
				? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
			rasterDesc.CullMode =
				desc.cullMode == GfxCullMode::None ? D3D12_CULL_MODE_NONE :
				desc.cullMode == GfxCullMode::Front ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK;
			rasterDesc.FrontCounterClockwise = desc.frontFace == GfxFrontFace::CCW;
			rasterDesc.DepthBias = desc.depthBias;
			rasterDesc.SlopeScaledDepthBias = desc.slopeScaledDepthBias;
			rasterDesc.DepthClipEnable = TRUE;
			return rasterDesc;
		}

		D3D12_DEPTH_STENCIL_DESC BuildDepthStencilDesc(const GfxGraphicsPipelineDesc& desc)
		{
			D3D12_DEPTH_STENCIL_DESC depthDesc = {};
			depthDesc.DepthEnable = desc.depthTest;
			depthDesc.DepthWriteMask = desc.depthWrite
				? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = Helpers::ToCompare(desc.depthCompare);
			return depthDesc;
		}
	}

	D3D12Pipeline::~D3D12Pipeline()
	{
		if (m_pipeline) 
			m_pipeline->Release();
	}

	void D3D12Device::CreateRootSignature()
	{
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
			makeSampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_WRAP),						// PointWrap
			makeSampler(1, D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_CLAMP),						// PointClamp
			makeSampler(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP),						// LinearWrap
			makeSampler(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),						// LinearClamp
			makeSampler(4, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR),						// LinearMirror
			makeSampler(5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						   D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),				// AnisoWrap
			makeSampler(6, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						   D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),				// AnisoClamp
			makeSampler(7, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER),						// LinearBorder
			makeSampler(8, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						   D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),				// ShadowCmp
			makeSampler(9, D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_BORDER),						// PointBorder
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
		HRESULT hr = D3D12SerializeVersionedRootSignature(&vdesc, &pBlob, &pError);
		CHECK_HR(hr, "ID3D12RootSignature - D3D12SerializeVersionedRootSignature");

		hr = m_device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature));
		CHECK_HR(hr, "ID3D12RootSignature - CreateRootSignature");

		pBlob->Release();
	}

	GfxPipeline* D3D12Device::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		const b8 bUsesMesh = desc.meshShader.pData != nullptr;

		auto* pipe = Allocator::Create<D3D12Pipeline>(CurrLoc());
		pipe->m_ownerDevice = this;
		HRESULT hr = S_OK;

		if (bUsesMesh)
		{
			MeshStateStream stream = {};
			stream.rootSignature.value = m_rootSignature;
			stream.taskShader.value = { desc.taskShader.pData, desc.taskShader.size };
			stream.meshShader.value = { desc.meshShader.pData, desc.meshShader.size };
			stream.pixelShader.value = { desc.pixelShader.pData, desc.pixelShader.size };
			stream.blend.value = BuildBlendDesc(desc);
			stream.sampleMask.value = UINT_MAX;
			stream.rasterizer.value = BuildRasterizerDesc(desc);
			stream.depthStencil.value = BuildDepthStencilDesc(desc);
			stream.topology.value = Helpers::ToTopologyType(desc.topology);
			stream.rtFormats.value.NumRenderTargets = desc.colorTargetCount;
			for (u32 i = 0; i < desc.colorTargetCount; i++)
				stream.rtFormats.value.RTFormats[i] = Helpers::ToDXGIFormat(desc.colorFormats[i]);
			stream.dsFormat.value = Helpers::ToDXGIFormat(desc.depthFormat);
			stream.sampleDesc.value = { 1, 0 };

			D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
			streamDesc.SizeInBytes = sizeof(stream);
			streamDesc.pPipelineStateSubobjectStream = &stream;

			hr = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipe->m_pipeline));
			CHECK_REASON(hr, "ID3D12PipelineState - CreatePipelineState (Mesh)");
		}
		else
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_rootSignature;
			psoDesc.VS = { desc.vertexShader.pData, desc.vertexShader.size };
			psoDesc.PS = { desc.pixelShader.pData, desc.pixelShader.size };
			psoDesc.InputLayout = { nullptr, 0 };
			psoDesc.BlendState = BuildBlendDesc(desc);
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.RasterizerState = BuildRasterizerDesc(desc);
			psoDesc.DepthStencilState = BuildDepthStencilDesc(desc);
			psoDesc.PrimitiveTopologyType = Helpers::ToTopologyType(desc.topology);
			psoDesc.NumRenderTargets = desc.colorTargetCount;
			for (u32 i = 0; i < desc.colorTargetCount; i++)
				psoDesc.RTVFormats[i] = Helpers::ToDXGIFormat(desc.colorFormats[i]);
			psoDesc.DSVFormat = Helpers::ToDXGIFormat(desc.depthFormat);
			psoDesc.SampleDesc = { 1, 0 };

			hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipe->m_pipeline));
			CHECK_REASON(hr, "ID3D12PipelineState - CreateGraphicsPipelineState");
		}

		if (FAILED(hr))
		{
			Allocator::Delete(pipe);
			return {};
		}

		pipe->m_type = GfxPipelineType::Graphics;
		pipe->m_topology = Helpers::ToTopology(desc.topology);
		pipe->m_usesMeshShading = bUsesMesh;

		return pipe;
	}

	GfxPipeline* D3D12Device::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.CS = { desc.computeShader.pData, desc.computeShader.size };

		auto* pipe = Allocator::Create<D3D12Pipeline>(CurrLoc());
		pipe->m_ownerDevice = this;

		HRESULT hr = m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipe->m_pipeline));
		CHECK_REASON(hr, "ID3D12PipelineState - CreateComputePipelineState");

		if (FAILED(hr))
		{
			Allocator::Delete(pipe);
			return {};
		}

		pipe->m_type = GfxPipelineType::Compute;

		return pipe;
	}
}