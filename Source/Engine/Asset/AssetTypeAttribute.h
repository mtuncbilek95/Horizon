#pragma once

#include <Engine/Asset/AssetOrigin.h>

#include <Runtime/RTTR/Reflection.h>

#include <initializer_list>
#include <string>

namespace Horizon::Engine
{
	class H_EXPORT AssetTypeAttribute : public Reflect::Attribute
	{
	public:
		AssetTypeAttribute(const std::string& name, u32 version, AssetOrigin origin, List<std::string> extensions) : m_origin(origin), m_extensions(extensions)
		{
		}

		const std::string& GetTypeName() const { return m_assetTypeName; }
		u32 GetVersion() const { return m_version; }
		AssetOrigin GetOrigin() const { return m_origin; }
		const List<std::string>& GetExtensions() const { return m_extensions; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<AssetTypeAttribute>(); }

	private:
		std::string m_assetTypeName;
		u32 m_version = 1;
		AssetOrigin m_origin;
		List<std::string> m_extensions;
	};
}