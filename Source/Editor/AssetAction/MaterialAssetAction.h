#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Material/Material.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Asset::Material>()]);
		class MaterialAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(MaterialAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}