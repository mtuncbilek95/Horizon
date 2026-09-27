#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Scene/SceneAsset.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Engine::SceneAsset>()])
	class H_EXPORT SceneAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(SceneAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}