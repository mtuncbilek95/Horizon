#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;

	class EDITOR_API DomainFile
	{
	public:
		static constexpr std::string_view MetaSuffix = ".hmeta";
		static constexpr std::string_view CookSuffix = ".hasset";

	public:
		DomainFile(DomainFolder* pParent, const std::string& name, const std::string& metaPath, 
			const std::string& sourcePath, const std::string& cookFolder, const std::string& assetType);
		~DomainFile();

		DomainFolder* GetParent() const { return m_parent; }

		const Guid& GetID() const { return m_id; }
		const std::string& GetAssetTypeName() { return m_assetTypeName; }

		const std::string& GetName() const { return m_name; }
		std::string GetExtension() const { return m_extension; }

		std::string GetRelativePath() const;
		const std::string& GetMetaPath() const { return m_metaPath; }
		const std::string& GetSourcePath() const { return m_sourcePath; }
		const std::string& GetCookFolder() const { return m_cookFolder; }
		std::string GetCookedPath() const { return m_cookFolder + "/" + m_id.ToString() + std::string(CookSuffix); }

		void Rename(const std::string& newName);

	private:
		DomainFolder* m_parent = nullptr;

		Guid m_id;
		std::string m_assetTypeName;

		std::string m_name;
		std::string m_extension;

		std::string m_metaPath;
		std::string m_sourcePath;
		std::string m_cookFolder;
	};
}