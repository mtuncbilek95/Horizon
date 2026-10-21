// ShaderAssetImportPipeline.cpp

#include "ShaderAssetImportPipeline.h"

#include <Engine/Asset/Shader/ShaderAssetProperties.h>
#include <Engine/Asset/Shader/ShaderAssetStage.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::Editor
{
	void ShaderAssetImportPipeline::PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
	}

	AssetImportResult ShaderAssetImportPipeline::Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != sizeof(Engine::ShaderAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "Shader properties block has a wrong size");
			return AssetImportResult::InvalidData;
		}

		Engine::ShaderAssetProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::ShaderAssetProperties));

		if (props.stageCount == 0 || bytes.table.GetCount() != props.stageCount * sizeof(Engine::ShaderAssetStage))
		{
			Terminal::Error(StringOps::GetName(this), "Shader table size does not match stageCount");
			return AssetImportResult::InvalidData;
		}

		b8 hasVertex = false;
		b8 hasPixel = false;

		auto* pStages = (const Engine::ShaderAssetStage*)(bytes.table.GetData());
		for (u32 i = 0; i < props.stageCount; ++i)
		{
			const Engine::ShaderAssetStage& stage = pStages[i];

			if (stage.byteSize == 0 || stage.offset + stage.byteSize > bytes.binary.GetCount())
			{
				Terminal::Error(StringOps::GetName(this), "Shader stage {} points outside of the byte code payload", i);
				return AssetImportResult::InvalidData;
			}

			hasVertex = hasVertex || stage.stage == RHI::GfxShaderStage::Vertex;
			hasPixel = hasPixel || stage.stage == RHI::GfxShaderStage::Pixel;
		}

		if (!hasVertex || !hasPixel)
		{
			Terminal::Error(StringOps::GetName(this), "Shader needs both a vertex and a pixel stage");
			return AssetImportResult::InvalidData;
		}

		return AssetImportResult::Success;
	}
}