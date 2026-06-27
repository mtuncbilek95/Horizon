#pragma once

#include <Editor/Domain/DomainObject.h>

#include <filesystem>
#include <string>

namespace Horizon
{
	class Engine;
	class DomainFile;
	class DomainFolder;

	struct DomainFolderDesc
	{
		std::filesystem::path folderPath;
		DomainFolder* pParent = nullptr;
	};

	class H_EXPORT DomainFolder : public DomainObject
	{
	public:
		DomainFolder(const DomainFolderDesc& desc, Engine* pEngine);
		~DomainFolder();

		const std::vector<DomainFolder*>& GetSubfolders() const { return m_subFolders; }
		const std::vector<DomainFile*>& GetFiles() const { return m_files; }

		DomainFolder* GetParentFolder() const { return m_parent; }

		const std::filesystem::path& GetPath() const { return m_path; }
		const std::string& GetName() const { return m_name; }

		b8 IsRoot() const { return m_parent == nullptr; }

		b8 HasFile(const std::string& fileName);
		b8 HasFolder(const std::string& folderName);

		DomainFile* FindFile(const std::string& name);
		DomainFolder* FindFolder(const std::string& name);

		void ResetChildMarks();
		void SweepUnmarked();

		void AddFile(DomainFile* pFile) { m_files.push_back(pFile); }
		void AddSubfolder(DomainFolder* pSub) { m_subFolders.push_back(pSub); }

	private:
		std::vector<DomainFolder*> m_subFolders;
		std::vector<DomainFile*> m_files;

		Engine* m_engine;
		DomainFolder* m_parent;

		std::string m_name;
		std::filesystem::path m_path;
	};
}