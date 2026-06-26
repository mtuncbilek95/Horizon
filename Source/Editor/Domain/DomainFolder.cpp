#include "DomainFolder.h"

#include <Editor/Domain/DomainFile.h>

namespace Horizon
{
	DomainFolder::DomainFolder(const DomainFolderDesc& desc, Engine* pEngine)
	{
	}

	DomainFolder::~DomainFolder()
	{
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
}