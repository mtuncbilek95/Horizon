#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Asset/AssetSerializerSettings.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <filesystem>
#include <string>

namespace Horizon
{
	class DomainFolder;

	struct DomainFileDesc
	{
		DomainFolder* parentFolder;
		
		std::filesystem::path metaPath;
		std::filesystem::path cookedPath;

		std::string assetTypeName;
		std::string name;

		Reflect::Type* assetType;
		Reflect::Type* settingsType;

		std::string settings;

		Guid guid;
		usize cookedSize;
	};

	class H_EXPORT DomainFile final
	{
		friend class DomainSystem;
	public:
		DomainFile(const DomainFileDesc& desc, Engine* pEngine);
		~DomainFile();

		Engine* GetEngine() const { return m_engine; }
		DomainFolder* GetParent() const { return m_parentFolder; }

		const std::filesystem::path& GetMetaPath() const { return m_metaPath; }
		const std::filesystem::path& GetCookedPath() const { return m_cookedPath; }

		const std::string& GetAssetTypeName() const { return m_assetTypeName; }
		const std::string& GetName() const { return m_name; }

		Reflect::Type* GetAssetType() const { return m_assetType; }

	private:
		Engine* m_engine;
		DomainFolder* m_parentFolder;

		std::filesystem::path m_metaPath;
		std::filesystem::path m_cookedPath;

		std::string m_assetTypeName;
		std::string m_name;

		Reflect::Type* m_assetType;
		Reflect::Type* m_settingsType;

		AssetEntry* m_assetEntry;
		AssetSerializerSettings* m_assetSettings;
		
		Guid m_id;
		usize m_cookedSize;

	};
}