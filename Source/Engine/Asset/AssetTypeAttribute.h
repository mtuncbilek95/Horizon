#pragma once

#include <Engine/Asset/AssetOrigin.h>

#include <Runtime/RTTR/Reflection.h>

#include <initializer_list>
#include <string>

namespace Horizon::Engine
{
	class H_EXPORT AssetTypeAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AssetTypeAttribute);
	public:
		AssetTypeAttribute(const std::string& name, u32 version, AssetOrigin origin) : m_assetTypeName(name),
			m_origin(origin), m_version(version)
		{
		}
		~AssetTypeAttribute() = default;

		const std::string& GetTypeName() const { return m_assetTypeName; }
		u32 GetVersion() const { return m_version; }
		AssetOrigin GetOrigin() const { return m_origin; }

	private:
		std::string m_assetTypeName;
		u32 m_version = 1;
		AssetOrigin m_origin;
	};
}