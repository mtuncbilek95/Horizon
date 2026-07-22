#pragma once

#include <Editor/Domain/ImportPipeline/ImportFileSettings.h>
#include <Editor/Domain/ImportPipeline/ImportFileAttribute.h>
#include <Engine/Asset/World/WorldAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS(ImportFileAttribute[Reflect::TypeOf<WorldAsset>()]);
	class H_EXPORT WorldImportSettings : public ImportFileSettings
	{
	public:
		void OnImportFrom() final {}
		void OnImportDefault(MetaHeader& outHeaderToSerialize) final;
	};
}