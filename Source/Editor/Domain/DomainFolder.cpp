#include "DomainFolder.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSystem.h>

#include <Editor/Domain/DomainFile.h>

namespace Horizon
{
	DomainFolder::DomainFolder(const DomainFolderDesc& desc, Engine* pEngine) : m_engine(pEngine), m_folderPath(desc.folderPath),
		m_relativePath(desc.relativePath), m_name(desc.folderName), m_parentFolder(desc.parentFolder)
	{
	}

	DomainFolder::~DomainFolder()
	{
		for (auto* file : m_files)
			Allocator::Delete(file);

		for (auto* folder : m_subFolders)
			Allocator::Delete(folder);
	}

	b8 DomainFolder::HasFile(const std::string& fileName)
	{
		for (auto* file : m_files)
		{
			if (file->GetName() == fileName)
				return true;
		}

		return false;
	}

	b8 DomainFolder::HasFolder(const std::string& folderName)
	{
		for (auto* folder : m_subFolders)
		{
			if (folder->GetName() == folderName)
				return true;
		}

		return false;
	}

	DomainFile* DomainFolder::FindFile(const std::string& name)
	{
		for (auto* file : m_files)
		{
			if (file->GetName() == name)
				return file;
		}

		return nullptr;
	}

	DomainFolder* DomainFolder::FindFolder(const std::string& name)
	{
		for (auto* folder : m_subFolders)
		{
			if (folder->GetName() == name)
				return folder;
		}

		return nullptr;
	}
}