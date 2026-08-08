#include "D3D12Pipeline.h"

#include <Runtime/Log/Terminal.h>
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
			D3D12_BLEND_DESC blendDesc = {};

			blendDesc.IndependentBlendEnable = desc.independentBlend;

			for (u32 i = 0; i < desc.colorTargetCount; i++)
			{
				const GfxBlendState& state = desc.independentBlend ? desc.blends[i] : desc.blends[0];
				D3D12_RENDER_TARGET_BLEND_DESC& target = blendDesc.RenderTarget[i];

				target.BlendEnable = state.enable;
				target.SrcBlend = Helpers::ToBlend(state.srcColor);
				target.DestBlend = Helpers::ToBlend(state.dstColor);
				target.BlendOp = Helpers::ToBlendOp(state.colorOp);
				target.SrcBlendAlpha = Helpers::ToBlend(state.srcAlpha);
				target.DestBlendAlpha = Helpers::ToBlend(state.dstAlpha);
				target.BlendOpAlpha = Helpers::ToBlendOp(state.alphaOp);
				target.RenderTargetWriteMask = Helpers::ToColorWriteMask(state.writeMask);
			}

			return blendDesc;
		}

		D3D12_RASTERIZER_DESC BuildRasterizerDesc(const GfxGraphicsPipelineDesc& desc)
		{
			D3D12_RASTERIZER_DESC rasterDesc = {};

			rasterDesc.FillMode = Helpers::ToFillMode(desc.rasterizer.fillMode);
			rasterDesc.CullMode = Helpers::ToCullMode(desc.rasterizer.cullMode);
			rasterDesc.FrontCounterClockwise = Helpers::ToFrontCounterClockwise(desc.rasterizer.frontFace);
			rasterDesc.DepthBias = desc.rasterizer.depthBias;
			rasterDesc.DepthBiasClamp = desc.rasterizer.depthBiasClamp;
			rasterDesc.SlopeScaledDepthBias = desc.rasterizer.slopeScaledDepthBias;
			rasterDesc.DepthClipEnable = TRUE;
			rasterDesc.MultisampleEnable = desc.sampleCount != GfxSampleCount::Count1;
			return rasterDesc;
		}

		D3D12_DEPTH_STENCILOP_DESC BuildStencilFace(const GfxStencilFace& face)
		{
			D3D12_DEPTH_STENCILOP_DESC faceDesc = {};

			faceDesc.StencilFailOp = Helpers::ToStencilOp(face.failOp);
			faceDesc.StencilDepthFailOp = Helpers::ToStencilOp(face.depthFailOp);
			faceDesc.StencilPassOp = Helpers::ToStencilOp(face.passOp);
			faceDesc.StencilFunc = Helpers::ToCompare(face.compareOp);
			return faceDesc;
		}

		D3D12_DEPTH_STENCIL_DESC BuildDepthStencilDesc(const GfxGraphicsPipelineDesc& desc)
		{
			D3D12_DEPTH_STENCIL_DESC depthDesc = {};

			depthDesc.DepthEnable = desc.depthStencil.depthTest;
			depthDesc.DepthWriteMask = desc.depthStencil.depthWrite
				? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = Helpers::ToCompare(desc.depthStencil.depthCompare);
			depthDesc.StencilEnable = desc.depthStencil.stencilTest;
			depthDesc.StencilReadMask = desc.depthStencil.stencilReadMask;
			depthDesc.StencilWriteMask = desc.depthStencil.stencilWriteMask;
			depthDesc.FrontFace = BuildStencilFace(desc.depthStencil.front);
			depthDesc.BackFace = BuildStencilFace(desc.depthStencil.back);
			return depthDesc;
		}
	}

	D3D12Pipeline::~D3D12Pipeline()
	{
		if (m_pipeline)
			m_pipeline->Release();
	}

	GfxPipeline* D3D12Device::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		const b8 bUsesMesh = desc.meshShader.IsValid();
		const DXGI_SAMPLE_DESC sampleDesc = { Helpers::ToSampleCount(desc.sampleCount), 0 };

		auto* pPipe = Memory::Allocator::Create<D3D12Pipeline>(Memory::CurrLoc());

		pPipe->m_ownerDevice = this;

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
			stream.sampleDesc.value = sampleDesc;

			D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};

			streamDesc.SizeInBytes = sizeof(stream);
			streamDesc.pPipelineStateSubobjectStream = &stream;

			hr = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pPipe->m_pipeline));
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
			psoDesc.SampleDesc = sampleDesc;

			hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPipe->m_pipeline));
			CHECK_REASON(hr, "ID3D12PipelineState - CreateGraphicsPipelineState");
		}

		if (FAILED(hr))
		{
			Memory::Allocator::Delete(pPipe);
			return nullptr;
		}

		pPipe->m_type = GfxPipelineType::Graphics;
		pPipe->m_topology = Helpers::ToTopology(desc.topology);
		pPipe->m_usesMeshShading = bUsesMesh;

		return pPipe;
	}

	GfxPipeline* D3D12Device::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.CS = { desc.computeShader.pData, desc.computeShader.size };

		auto* pPipe = Memory::Allocator::Create<D3D12Pipeline>(Memory::CurrLoc());

		pPipe->m_ownerDevice = this;

		HRESULT hr = m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pPipe->m_pipeline));
		CHECK_REASON(hr, "ID3D12PipelineState - CreateComputePipelineState");

		if (FAILED(hr))
		{
			Memory::Allocator::Delete(pPipe);
			return nullptr;
		}

		pPipe->m_type = GfxPipelineType::Compute;

		return pPipe;
	}
}