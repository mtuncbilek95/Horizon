#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT AssetTypeAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AssetTypeAttribute);
	public:
		AssetTypeAttribute(Reflect::TypeHandle assetType) : m_assetType(assetType)
		{
		}
		~AssetTypeAttribute() = default;

		Reflect::TypeHandle GetAssetType() const { return m_assetType; }

	private:
		Reflect::TypeHandle m_assetType;
	};
}