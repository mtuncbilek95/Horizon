#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
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

	GfxPipeline* Gfx::CreateGfxGraphicsPipeline(GfxDevice* pContext, GfxPipelineLayout* pLayout,
		const GfxGraphicsPipelineDesc& desc)
	{
		const b8 bUsesMesh = desc.meshShader.pData != nullptr;

		GfxPipeline* pPipeline = new GfxPipeline();
		HRESULT bResult = S_OK;

		if (bUsesMesh)
		{
			MeshStateStream stream = {};
			stream.rootSignature.value = pLayout->pLayout;
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

			bResult = pContext->pDevice->CreatePipelineState(&streamDesc,
				IID_PPV_ARGS(&pPipeline->pPipeline));
			CHECK_REASON(bResult, "ID3D12PipelineState - CreatePipelineState (Mesh)");
		}
		else
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = pLayout->pLayout;
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

			bResult = pContext->pDevice->CreateGraphicsPipelineState(&psoDesc,
				IID_PPV_ARGS(&pPipeline->pPipeline));
			CHECK_REASON(bResult, "ID3D12PipelineState - CreateGraphicsPipelineState");
		}

		if (FAILED(bResult))
		{
			delete pPipeline;
			return nullptr;
		}

		pPipeline->topology = Helpers::ToTopology(desc.topology);
		pPipeline->bIsCompute = false;
		pPipeline->bUsesMeshShading = bUsesMesh;

		return pPipeline;
	}

	GfxPipeline* Gfx::CreateGfxComputePipeline(GfxDevice* pContext, GfxPipelineLayout* pLayout,
		const GfxComputePipelineDesc& desc)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = pLayout->pLayout;
		psoDesc.CS = { desc.computeShader.pData, desc.computeShader.size };

		GfxPipeline* pPipeline = new GfxPipeline();
		HRESULT bResult = pContext->pDevice->CreateComputePipelineState(&psoDesc,
			IID_PPV_ARGS(&pPipeline->pPipeline));
		CHECK_REASON(bResult, "ID3D12PipelineState - CreateComputePipelineState");

		if (FAILED(bResult))
		{
			delete pPipeline;
			return nullptr;
		}

		pPipeline->bIsCompute = true;
		return pPipeline;
	}

	void Gfx::DestroyGfxPipeline(GfxPipeline* plHandl)
	{
		if (plHandl->pPipeline)
			plHandl->pPipeline->Release();

		delete plHandl;
	}
}