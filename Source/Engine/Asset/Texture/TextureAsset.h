#pragma once

#include <Engine/Asset/AssetObject.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API TextureAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(TextureAsset);
	public:
		TextureAsset() = default;
		~TextureAsset() = default;

	protected:
		void OnPropertiesLoaded(ReadOnlyList<u8> propertyBytes) {}
	};
}