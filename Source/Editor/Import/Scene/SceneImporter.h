#pragma once

#include <Editor/Import/AssetImporter.h>
#include <Editor/Import/ImportTypeAttribute.h>
#include <Engine/Asset/Scene/SceneAsset.h>

namespace Horizon::Editor
{
	HCLASS(ImportType[Reflect::TypeOf<Engine::SceneAsset>(), { ".hscene" }]);
	class H_EXPORT SceneImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(SceneImporter);
	public:
		b8 Import(const ImportRequest& request, ImportSink& sink) final;
	};
}