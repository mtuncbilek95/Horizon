#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/TypeAttribute.h>

#include <string_view>

namespace Horizon
{
	class AssetLoadStrategyAttribute : public TypeAttribute
	{
	public:
		AssetLoadStrategyAttribute(std::string_view assetType, b8 isDefault = false)
			: m_assetType(assetType), m_isDefault(isDefault)
		{
		}

		std::string_view GetAssetType() const { return m_assetType; }
		b8 IsDefault() const { return m_isDefault; }

		ReflectionTypeHandle GetTypeId() const final { return TypeIdOf<AssetLoadStrategyAttribute>(); }

	private:
		std::string_view m_assetType;
		b8 m_isDefault = false;
	};
}