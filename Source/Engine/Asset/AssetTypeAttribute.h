#pragma once

#include <Engine/Asset/AssetOrigin.h>

#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon
{
	class H_EXPORT AssetTypeAttribute : public Reflect::Attribute
	{
	public:
		AssetTypeAttribute(AssetOrigin origin, std::string extensions) : m_origin(origin), m_extensions(extensions)
		{
		}

		AssetOrigin GetOrigin() const { return m_origin; }
		const std::string& GetExtensions() const { return m_extensions; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<AssetTypeAttribute>(); }

	private:
		AssetOrigin m_origin;
		std::string m_extensions;
	};
}