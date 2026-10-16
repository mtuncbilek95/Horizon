#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;
	class AssetImportSettings;

	class EDITOR_API DomainFile
	{
	public:
		static constexpr std::string_view MetaSuffix = ".hmeta";
		static constexpr std::string_view CookSuffix = ".hasset";
		
		struct DomainSubAsset
		{
			std::string key;
			Guid id;
			Reflect::Type* pAssetType = nullptr;
		};

	public:
		DomainFile(DomainFolder* pParent, const Guid& id, const std::string& name, const std::string& metaPath, 
			const std::string& sourcePath, const std::string& cookFolder, Reflect::Type* pAssetType);
		~DomainFile();

		DomainFolder* GetParent() const { return m_parent; }

		const Guid& GetID() const { return m_id; }
		Reflect::Type* GetAssetType() { return m_assetType; }

		const std::string& GetName() const { return m_name; }
		std::string GetExtension() const { return m_extension; }

		std::string GetRelativePath() const;
		const std::string& GetMetaPath() const { return m_metaPath; }
		const std::string& GetSourcePath() const { return m_sourcePath; }
		const std::string& GetCookFolder() const { return m_cookFolder; }
		std::string GetCookedPath() const { return m_cookFolder + "/" + m_id.ToString() + std::string(CookSuffix); }

		const List<DomainSubAsset>& GetSubAssets() const { return m_subAssets; }
		DomainSubAsset* FindSubAsset(std::string_view key);
		std::string GetSubAssetCookedPath(const Guid& id) const { return m_cookFolder + "/" + id.ToString() + std::string(CookSuffix); }

		void Rename(const std::string& newName);
		
		b8 CreateSettings(Reflect::Type* pType);
		AssetImportSettings* GetSettings() const { return m_settings; }
		void ReleaseSettings();

	private:
		DomainFolder* m_parent = nullptr;

		Guid m_id;
		Reflect::Type* m_assetType;
		AssetImportSettings* m_settings = nullptr;

		std::string m_name;
		std::string m_extension;

		std::string m_metaPath;
		std::string m_sourcePath;
		std::string m_cookFolder;

		List<DomainSubAsset> m_subAssets;
	};
}