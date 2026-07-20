#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <filesystem>
#include <string>
#include <vector>
#include <span>

namespace Horizon
{
	class Engine;
	class DomainFile;
	class DomainFolder;

	struct DomainFolderDesc
	{
		DomainFolder* parentFolder;
		std::filesystem::path folderPath;
		std::filesystem::path relativePath;
		std::string folderName;
	};

	class H_EXPORT DomainFolder final
	{
		friend class DomainSystem;
	public:
		DomainFolder(const DomainFolderDesc& desc, Engine* pEngine);
		~DomainFolder();

		Engine* GetEngine() const { return m_engine; }

		const std::filesystem::path& GetFolderPath() const { return m_folderPath; }
		const std::filesystem::path& GetRelativePath() const { return m_relativePath; }
		const std::string& GetName() const { return m_name; }
		
		std::span<DomainFolder* const> GetSubFolders() const { return m_subFolders; }
		std::span<DomainFile* const> GetFiles() const { return m_files; }

		DomainFolder* GetParentFolder() const { return m_parentFolder; }

		b8 IsRoot() const { return !m_parentFolder; }
		b8 HasFile(const std::string& fileName);
		b8 HasFolder(const std::string& folderName);
		DomainFile* FindFile(const std::string& name);
		DomainFolder* FindFolder(const std::string& name);

	private:
		Engine* m_engine;

		std::filesystem::path m_folderPath;
		std::filesystem::path m_relativePath; // This needs to be there to refer asset properly in metadata.
		std::string m_name;

		std::vector<DomainFolder*> m_subFolders;
		std::vector<DomainFile*> m_files;

		DomainFolder* m_parentFolder;
	};
}