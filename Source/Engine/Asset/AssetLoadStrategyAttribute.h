#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <string_view>

namespace Horizon
{
	class AssetLoadStrategyAttribute : public Reflect::Attribute
	{
	public:
		AssetLoadStrategyAttribute(std::string_view assetType, b8 isDefault = false)
			: m_assetType(assetType), m_isDefault(isDefault)
		{
		}

		std::string_view GetAssetType() const { return m_assetType; }
		b8 IsDefault() const { return m_isDefault; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<AssetLoadStrategyAttribute>(); }

	private:
		std::string_view m_assetType;
		b8 m_isDefault = false;
	};
}