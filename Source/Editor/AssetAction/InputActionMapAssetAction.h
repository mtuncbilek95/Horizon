#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Input/InputActionMap.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Asset::InputActionMap>()]);
	class InputActionMapAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(InputActionMapAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}
