#pragma once

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/AssetAction/AssetAction.h>
#include <Engine/Asset/Texture/Texture.h>

namespace Horizon::Editor
{
	HCLASS(ActionType[Reflect::TypeOf<Asset::Texture>()]);
		class TextureAssetAction : public AssetAction
	{
		HORIZON_TYPE_REFLECT(TextureAssetAction);
	public:
		void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) final;
	};
}