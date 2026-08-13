#pragma once

#include <Editor/Domain/DomainFile.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon::Editor
{
	class H_EXPORT DomainFolder
	{
		friend class DomainSystem;
	public:
		DomainFolder(DomainFolder* pParent, const std::string& name, const std::string& absolutePath) : m_parent(pParent),
			m_name(name), m_absolutePath(absolutePath)
		{
		}

		~DomainFolder()
		{
			for (auto* folder : m_folders)
				Memory::Allocator::Delete(folder);

			for (auto* file : m_files)
				Memory::Allocator::Delete(file);
		}

		DomainFolder* GetParent() const { return m_parent; }
		b8 IsRoot() const { return m_parent == nullptr; }

		const std::string& GetName() const { return m_name; }
		const std::string& GetAbsolutePath() const { return m_absolutePath; }

		const List<DomainFolder*>& GetFolders() const { return m_folders; }
		const List<DomainFile*>& GetFiles() const { return m_files; }

		void Rename(const std::string& newName);

	private:
		DomainFolder* m_parent = nullptr;
		std::string m_name;
		std::string m_absolutePath;

		List<DomainFolder*> m_folders;
		List<DomainFile*> m_files;
	};
}