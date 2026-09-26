#pragma once

#include <Editor/Domain/DomainFile.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT DomainFolder
	{
	public:
		DomainFolder(DomainFolder* pParent, const std::string& name, const std::string& absolutePath, const std::string& cookPath) : m_parent(pParent),
			m_name(name), m_absolutePath(absolutePath), m_connectedCookPath(cookPath)
		{
		}

		~DomainFolder()
		{
			Clear();
		}

		DomainFolder* GetParent() const { return m_parent; }
		b8 IsRoot() const { return m_parent == nullptr; }

		const std::string& GetName() const { return m_name; }
		const std::string& GetAbsolutePath() const { return m_absolutePath; }
		std::string GetRelativePath() const;

		const List<DomainFolder*>& GetFolders() const { return m_folders; }
		const List<DomainFile*>& GetFiles() const { return m_files; }

		DomainFolder* FindFolder(std::string_view name) const;
		DomainFile* FindFile(std::string_view name) const;
		DomainFolder* ResolveFolder(std::string_view relativePath) const;

		DomainFolder* AddFolder(const std::string& name);
		DomainFile* AddFile(const std::string& name);
		b8 RemoveEntry(std::string_view name);

		void Refresh(List<std::string>& outSources);
		void Clear();

		void Rename(const std::string& newName);

	private:
		DomainFolder* m_parent = nullptr;
		std::string m_name;
		std::string m_absolutePath;

		std::string m_connectedCookPath;

		List<DomainFolder*> m_folders;
		List<DomainFile*> m_files;
	};
}
