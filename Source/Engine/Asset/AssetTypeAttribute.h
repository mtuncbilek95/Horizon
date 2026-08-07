#pragma once

#include <Engine/Asset/AssetOrigin.h>

#include <Runtime/RTTR/Reflection.h>

#include <initializer_list>
#include <string>

namespace Horizon
{
	class H_EXPORT AssetTypeAttribute : public Reflect::Attribute
	{
	public:
		AssetTypeAttribute(AssetOrigin origin, List<std::string> extensions) : m_origin(origin), m_extensions(extensions)
		{
		}

		AssetOrigin GetOrigin() const { return m_origin; }
		const List<std::string>& GetExtensions() const { return m_extensions; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<AssetTypeAttribute>(); }

	private:
		AssetOrigin m_origin;
		List<std::string> m_extensions;
	};
}