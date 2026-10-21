#pragma once

#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipelineContext.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	struct ImportProductBytes
	{
		List<u8> properties;
		List<u8> table;
		List<u8> binary;
	};

	class EDITOR_API ImportPipeline : public Reflect::Base
	{
	public:
		AssetImportResult Cook(const ImportPipelineContext& ctx, const Guid& id, const Reflect::Type* pAssetType, const std::string& cookedPath, ImportProductBytes& bytes);

	protected:
		virtual void PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes) {}
		virtual AssetImportResult Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes) = 0;

	private:
		b8 Write(const std::string& cookedPath, const Guid& id, const Reflect::Type* pAssetType, const ImportProductBytes& bytes);
	};
}