#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class H_EXPORT AssetRefAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AssetRefAttribute);
	public:
		AssetRefAttribute(TypeHandle assetType) : m_assetType(assetType)
		{
		}
		~AssetRefAttribute() = default;

		TypeHandle GetAssetType() const { return m_assetType; }

	private:
		TypeHandle m_assetType;
	};
}