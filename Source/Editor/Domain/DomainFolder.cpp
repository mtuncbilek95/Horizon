#include "DomainFolder.h"

#include <Runtime/Containers/StringHelper.h>
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
			if (StringHelper::CheckIfEquals(pFolder->GetName(), name))
				return pFolder;
		}

		return nullptr;
	}

	DomainFile* DomainFolder::FindFile(std::string_view name) const
	{
		std::string cleanName = std::string(name.substr(0, name.rfind('.')));

		for (DomainFile* pFile : m_files)
		{
			if (StringHelper::CheckIfEquals(pFile->GetName(), cleanName))
				return pFile;
		}

		return nullptr;
	}

	DomainFile* DomainFolder::FindFileWithExtension(std::string_view nameWithExtension) const
	{
		const usize dot = nameWithExtension.rfind('.');
		const std::string_view stem = dot == std::string_view::npos ? nameWithExtension : nameWithExtension.substr(0, dot);
		const std::string_view extension = dot == std::string_view::npos ? std::string_view() : nameWithExtension.substr(dot);

		for (DomainFile* pFile : m_files)
		{
			if (!StringHelper::CheckIfEquals(pFile->GetName(), stem))
				continue;

			if (!StringHelper::CheckIfEquals(pFile->GetExtension(), extension))
				continue;

			return pFile;
		}

		return nullptr;
	}

	DomainFolder* DomainFolder::ConfirmAndFindFolder(std::string_view relativePath) const
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

		DomainFolder* pChild = new DomainFolder(this, name,
			m_absolutePath + "/" + name, m_connectedCookPath);

		usize index = 0;

		while (index < m_folders.GetCount() && m_folders[index]->GetName() < name)
			++index;

		m_folders.PushAt(index, pChild);

		return pChild;
	}

	DomainFile* DomainFolder::AddFile(const Guid& id, const std::string& name, Reflect::Type* pAssetType)
	{
		DomainFile* pExisting = FindFile(name);

		if (pExisting != nullptr)
			return pExisting;

		const std::string absolutePath = m_absolutePath + "/" + name;

		DomainFile* pFile = new DomainFile(this, id, name,
			absolutePath + std::string(DomainFile::MetaSuffix), absolutePath, m_connectedCookPath, pAssetType);

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
			if (!StringHelper::CheckIfEquals(m_folders[i]->GetName(), name))
				continue;

			delete m_folders[i];
			m_folders.RemoveAt(i);
			return true;
		}

		for (usize i = 0; i < m_files.GetCount(); ++i)
		{
			if (!StringHelper::CheckIfEquals(m_files[i]->GetName(), name))
				continue;

			delete m_files[i];
			m_files.RemoveAt(i);
			return true;
		}

		return false;
	}

	b8 DomainFolder::RemoveFile(DomainFile* pFile)
	{
		for (usize i = 0; i < m_files.GetCount(); ++i)
		{
			if (m_files[i] != pFile)
				continue;

			delete m_files[i];
			m_files.RemoveAt(i);
			return true;
		}

		return false;
	}

	void DomainFolder::Refresh(List<std::string>& outSources)
	{
		Clear();

		const std::string relativePath = GetRelativePath();
		const List<PAL::Directory::Entry> entries = PAL::Directory::Iterate(m_absolutePath);

		for (const PAL::Directory::Entry& entry : entries)
		{
			if (entry.isDirectory)
			{
				AddFolder(entry.name)->Refresh(outSources);
				continue;
			}

			if (entry.name.ends_with(DomainFile::MetaSuffix))
				continue;

			if (relativePath.empty())
				outSources.PushBack(entry.name);
			else
				outSources.PushBack(relativePath + "/" + entry.name);
		}
	}

	void DomainFolder::Clear()
	{
		for (DomainFolder* pFolder : m_folders)
			delete pFolder;

		m_folders.Clear();

		for (DomainFile* pFile : m_files)
			delete pFile;

		m_files.Clear();
	}

	void DomainFolder::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::Directory::Rename(m_absolutePath, newPath);
	}
}
