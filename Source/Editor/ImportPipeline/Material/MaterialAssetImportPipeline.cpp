#include "MaterialAssetImportPipeline.h"

#include <Engine/Asset/Material/MaterialAssetProperties.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::Editor
{
	void MaterialAssetImportPipeline::PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
	}

	AssetImportResult MaterialAssetImportPipeline::Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != sizeof(Engine::MaterialAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "Material properties block has a wrong size");
			return AssetImportResult::InvalidData;
		}

		Engine::MaterialAssetProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::MaterialAssetProperties));

		if (bytes.table.GetCount() != props.valueCount * sizeof(Engine::MaterialParamValue))
		{
			Terminal::Error(StringOps::GetName(this), "Material table size does not match valueCount");
			return AssetImportResult::InvalidData;
		}

		if (!bytes.binary.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "Material carries a payload, it should not have any");
			return AssetImportResult::InvalidData;
		}

		return AssetImportResult::Success;
	}
}