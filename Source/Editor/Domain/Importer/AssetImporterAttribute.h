#pragma once

#include <Runtime/Reflection/TypeAttribute.h>

namespace Horizon
{
	class Engine;

	class AssetImporterAttribute : public TypeAttribute
	{
	public:
		AssetImporterAttribute(std::vector<std::string_view> assetExts, std::string_view defaultName, usize version) : m_assetExtensions(assetExts),
			m_defaultName(defaultName), m_version(version)
		{
		}
		~AssetImporterAttribute() = default;

		const std::vector<std::string_view>& GetAssetExtension() const { return m_assetExtensions; }
		std::string_view GetDefaultName() const { return m_defaultName; }
		usize GetVersion() const { return m_version; }

		ReflectionTypeHandle GetTypeId() const final { return TypeIdOf<AssetImporterAttribute>(); }

	private:
		std::vector<std::string_view> m_assetExtensions;
		std::string_view m_defaultName;
		usize m_version = 0;
	};
}