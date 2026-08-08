#pragma once

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetOrigin::Imported, {".jpeg", ".png"}])
	class H_EXPORT TextureAsset
	{

	};
}