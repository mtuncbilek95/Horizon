#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetTypeAttribute.h>

namespace Horizon::Engine
{
	HCLASS(AssetType["Text", 1, AssetOrigin::Imported | AssetOrigin::Generated]);
		class H_EXPORT TextAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(TextAsset);
	public:

	private:
	};
}