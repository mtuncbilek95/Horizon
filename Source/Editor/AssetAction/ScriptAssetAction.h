#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Script/ScriptAsset.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Engine::ScriptAsset>()])
		class EDITOR_API ScriptAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(ScriptAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}