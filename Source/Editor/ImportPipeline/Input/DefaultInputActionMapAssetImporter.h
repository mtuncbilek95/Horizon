#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Input/InputAction.h>
#include <Engine/Asset/Input/InputActionMap.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Asset::InputActionMap>(), { ".hinputmap" }]);
	class DefaultInputActionMapAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DefaultInputActionMapAssetImporter);
	public:
		DefaultInputActionMapAssetImporter() = default;
		~DefaultInputActionMapAssetImporter();

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

	private:
		List<Asset::InputAction> m_actions;
		b8 m_parsed = false;
	};
}
