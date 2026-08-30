#include "DomainFolder.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/PAL/File/Directory.h>

namespace Horizon::Editor
{
	std::string DomainFolder::GetRelativePath() const
	{
		if (m_parent == nullptr)
			return std::string();

		const std::string parentPath = m_parent->GetRelativePath();

		if (parentPath.empty())
			return m_name;

		return parentPath + "/" + m_name;
	}

	DomainFolder* DomainFolder::FindFolder(std::string_view name) const
	{
		for (DomainFolder* pFolder : m_folders)
		{
			if (StringOps::EqualsNoCase(pFolder->GetName(), name))
				return pFolder;
		}

		return nullptr;
	}

	DomainFile* DomainFolder::FindFile(std::string_view name) const
	{
		for (DomainFile* pFile : m_files)
		{
			if (StringOps::EqualsNoCase(pFile->GetName(), name))
				return pFile;
		}

		return nullptr;
	}

	DomainFolder* DomainFolder::ResolveFolder(std::string_view relativePath) const
	{
		const DomainFolder* pCurrent = this;
		usize offset = 0;

		while (offset < relativePath.size())
		{
			const usize separator = relativePath.find('/', offset);
			const usize count = separator == std::string_view::npos ? relativePath.size() - offset : separator - offset;

			pCurrent = pCurrent->FindFolder(relativePath.substr(offset, count));

			if (pCurrent == nullptr)
				return nullptr;

			offset = separator == std::string_view::npos ? relativePath.size() : separator + 1;
		}

		return const_cast<DomainFolder*>(pCurrent);
	}

	DomainFolder* DomainFolder::AddFolder(const std::string& name)
	{
		DomainFolder* pExisting = FindFolder(name);

		if (pExisting != nullptr)
			return pExisting;

		DomainFolder* pChild = Memory::Allocator::Create<DomainFolder>(Memory::CurrLoc(), this, name,
			m_absolutePath + "/" + name);

		usize index = 0;

		while (index < m_folders.GetCount() && m_folders[index]->GetName() < name)
			++index;

		m_folders.PushAt(index, pChild);

		return pChild;
	}

	DomainFile* DomainFolder::AddFile(const std::string& name)
	{
		DomainFile* pExisting = FindFile(name);

		if (pExisting != nullptr)
			return pExisting;

		const std::string absolutePath = m_absolutePath + "/" + name;

		DomainFile* pFile = Memory::Allocator::Create<DomainFile>(Memory::CurrLoc(), this, name,
			absolutePath + std::string(DomainFile::MetaSuffix), absolutePath);

		usize index = 0;

		while (index < m_files.GetCount() && m_files[index]->GetName() < name)
			++index;

		m_files.PushAt(index, pFile);

		return pFile;
	}

	b8 DomainFolder::RemoveEntry(std::string_view name)
	{
		for (usize i = 0; i < m_folders.GetCount(); ++i)
		{
			if (!StringOps::EqualsNoCase(m_folders[i]->GetName(), name))
				continue;

			Memory::Allocator::Delete(m_folders[i]);
			m_folders.RemoveAt(i);
			return true;
		}

		for (usize i = 0; i < m_files.GetCount(); ++i)
		{
			if (!StringOps::EqualsNoCase(m_files[i]->GetName(), name))
				continue;

			Memory::Allocator::Delete(m_files[i]);
			m_files.RemoveAt(i);
			return true;
		}

		return false;
	}

	void DomainFolder::Refresh()
	{
		Clear();

		const List<PAL::Directory::Entry> entries = PAL::Directory::Iterate(m_absolutePath);

		for (const PAL::Directory::Entry& entry : entries)
		{
			if (!entry.isDirectory)
			{
				if (entry.name.ends_with(DomainFile::MetaSuffix))
					continue;

				AddFile(entry.name);
				continue;
			}

			AddFolder(entry.name)->Refresh();
		}
	}

	void DomainFolder::Clear()
	{
		for (DomainFolder* pFolder : m_folders)
			Memory::Allocator::Delete(pFolder);

		m_folders.Clear();

		for (DomainFile* pFile : m_files)
			Memory::Allocator::Delete(pFile);

		m_files.Clear();
	}

	void DomainFolder::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::Directory::Rename(m_absolutePath, newPath);
	}
}