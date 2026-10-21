#pragma once

#include <Editor/ImportPipeline/Material/MaterialParamEntry.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Shader/ShaderAsset.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API MaterialDocument : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(MaterialDocument);
	public:
		MaterialDocument() = default;
		~MaterialDocument() = default;

		HFIELD(AssetRef[Reflect::TypeOf<Engine::ShaderAsset>()]);
		Engine::AssetHandle<Engine::ShaderAsset> m_shader;

		HFIELD();
		List<MaterialParamEntry> m_params;
	};
}