#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Mesh/Mesh.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Asset::Mesh>()]);
	class MeshAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(MeshAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}
