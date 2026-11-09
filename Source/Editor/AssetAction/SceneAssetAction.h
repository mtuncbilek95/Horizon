#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Scene/Scene.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Asset::Scene>()])
	class SceneAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(SceneAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}