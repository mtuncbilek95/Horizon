#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetTypeAttribute.h>

namespace Horizon::Engine
{
	HCLASS(AssetType["Texture2D", 1, AssetOrigin::Imported]);
	class H_EXPORT Texture2DAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(Texture2DAsset);

	private:
	};
}