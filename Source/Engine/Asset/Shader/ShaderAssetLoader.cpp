#include "ShaderAssetLoader.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Shader/ShaderAssetStage.h>
#include <Engine/Rendering/ForwardPassFormats.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/RHI/Pipeline/GfxGraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>

#include <cstring>

namespace Horizon::Engine
{
	void ShaderAssetLoader::LoadAsync(ShaderAsset* pAsset)
	{
	}

	void ShaderAssetLoader::UnloadAsync(ShaderAsset* pAsset)
	{
	}

	void ShaderAssetLoader::LoadSync(ShaderAsset* pAsset)
	{
		if (m_context == nullptr || m_context->GetDevice() == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "No device! Shaders cannot be created", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		RHI::GfxDevice* pDevice = m_context->GetDevice();

		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		List<u8> headerBytes;
		if (!PAL::File::ReadMemory(fileHandl, headerBytes, 0, sizeof(AssetHeader)) || headerBytes.GetCount() < sizeof(AssetHeader))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the AssetHeader", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		AssetHeader header;
		std::memcpy(&header, headerBytes.GetData(), sizeof(AssetHeader));

		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.propertySize != sizeof(ShaderAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "ShaderAssetProperties are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> propertyBytes;
		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for ShaderAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		ShaderAssetProperties props;
		std::memcpy(&props, propertyBytes.GetData(), sizeof(ShaderAssetProperties));

		if (props.stageCount == 0 || props.stageCount * sizeof(ShaderAssetStage) != header.tableSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Table size does not match the stage count", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> tableBytes;
		if (!PAL::File::ReadMemory(fileHandl, tableBytes, header.tableOffset, header.tableOffset + header.tableSize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the stage table", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> payloadBytes;
		if (!PAL::File::ReadMemory(fileHandl, payloadBytes, header.payloadOffset, header.payloadOffset + header.payloadSize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for the shader byte code", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		PAL::File::ReleaseAccess(fileHandl);

		List<ShaderAssetStage> stages(props.stageCount);
		std::memcpy(stages.GetData(), tableBytes.GetData(), header.tableSize);

		const ShaderAssetStage* pVertexStage = nullptr;
		const ShaderAssetStage* pPixelStage = nullptr;

		for (const ShaderAssetStage& stage : stages)
		{
			if (stage.byteSize == 0 || stage.offset + stage.byteSize > payloadBytes.GetCount())
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "a stage points outside of the byte code payload", pAsset->m_id.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				return;
			}

			if (stage.stage == RHI::GfxShaderStage::Vertex)
				pVertexStage = &stage;
			else if (stage.stage == RHI::GfxShaderStage::Pixel)
				pPixelStage = &stage;
		}

		if (!pVertexStage || !pPixelStage)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Vertex or pixel stage is missing", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		RHI::GfxShaderDesc vertexDesc = {};
		vertexDesc.stage = RHI::GfxShaderStage::Vertex;
		vertexDesc.pByteCode = payloadBytes.GetData() + pVertexStage->offset;
		vertexDesc.byteCodeSize = pVertexStage->byteSize;

		RHI::GfxShader* pVertexShader = pDevice->CreateShader(vertexDesc);
		if (!pVertexShader)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Vertex shader could not be created", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		RHI::GfxShaderDesc pixelDesc = {};
		pixelDesc.stage = RHI::GfxShaderStage::Pixel;
		pixelDesc.pByteCode = payloadBytes.GetData() + pPixelStage->offset;
		pixelDesc.byteCodeSize = pPixelStage->byteSize;

		RHI::GfxShader* pPixelShader = pDevice->CreateShader(pixelDesc);
		if (!pPixelShader)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Pixel shader could not be created", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			Memory::Allocator::Delete(pVertexShader);
			return;
		}

		const MaterialRenderState& state = props.renderState;
		const b8 isTranslucent = state.blendMode == MaterialBlendMode::Translucent;

		RHI::GfxGraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pVertexShader = pVertexShader;
		pipelineDesc.pPixelShader = pPixelShader;
		pipelineDesc.colorFormats[0] = ForwardPassFormats::Color;
		pipelineDesc.colorTargetCount = 1;
		pipelineDesc.depthFormat = ForwardPassFormats::Depth;
		pipelineDesc.topology = RHI::GfxPrimitiveTopology::TriangleList;

		pipelineDesc.rasterizer.fillMode = RHI::GfxFillMode::Solid;
		pipelineDesc.rasterizer.cullMode = state.twoSided ? RHI::GfxCullMode::None : RHI::GfxCullMode::Back;
		pipelineDesc.rasterizer.frontFace = RHI::GfxFrontFace::CCW;

		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = !isTranslucent;
		pipelineDesc.depthStencil.depthCompare = RHI::GfxCompareOp::Greater;

		if (isTranslucent)
		{
			pipelineDesc.blends[0].enable = true;
			pipelineDesc.blends[0].srcColor = RHI::GfxBlendFactor::SrcAlpha;
			pipelineDesc.blends[0].dstColor = RHI::GfxBlendFactor::InvSrcAlpha;
			pipelineDesc.blends[0].srcAlpha = RHI::GfxBlendFactor::One;
			pipelineDesc.blends[0].dstAlpha = RHI::GfxBlendFactor::InvSrcAlpha;
		}

		RHI::GfxPipeline* pPipeline = pDevice->CreatePipeline(pipelineDesc);

		Memory::Allocator::Delete(pPixelShader);
		Memory::Allocator::Delete(pVertexShader);

		if (!pPipeline)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Pipeline could not be created", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_properties = props;
		pAsset->m_pipeline = pPipeline;
		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void ShaderAssetLoader::UnloadSync(ShaderAsset* pAsset)
	{
		m_context->GetDevice()->WaitIdle();
		ReleaseAsset(pAsset);
	}

	void ShaderAssetLoader::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
		m_context = GetEngine()->RequestContext<GraphicsContext>();

		if (m_context == nullptr)
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, shaders cannot be loaded");
	}

	void ShaderAssetLoader::OnRegisteredAsset(const AssetEntry& entry)
	{
		auto* pShaderAsset = static_cast<ShaderAsset*>(entry.pAssetObject);

		pShaderAsset->m_id = entry.assetId;
		pShaderAsset->m_cookPath = entry.cookPath;
	}

	void ShaderAssetLoader::OnUnregisteredAsset(const AssetEntry& entry)
	{
		auto* pShaderAsset = static_cast<ShaderAsset*>(entry.pAssetObject);

		if (pShaderAsset->m_pipeline == nullptr)
		{
			pShaderAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
			return;
		}

		m_context->GetDevice()->WaitIdle();
		ReleaseAsset(pShaderAsset);
	}

	void ShaderAssetLoader::OnRefreshAsset(const AssetEntry& entry)
	{
		auto* pShaderAsset = static_cast<ShaderAsset*>(entry.pAssetObject);

		if (pShaderAsset->m_pipeline == nullptr)
		{
			pShaderAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
			return;
		}

		m_context->GetDevice()->WaitIdle();
		ReleaseAsset(pShaderAsset);
	}

	void ShaderAssetLoader::ReleaseAsset(ShaderAsset* pAsset)
	{
		if (pAsset->m_pipeline)
			Memory::Allocator::Delete(pAsset->m_pipeline);

		pAsset->m_pipeline = nullptr;
		pAsset->m_properties = {};
		pAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}
}