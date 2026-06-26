#include "DomainFolder.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSubsystem.h>

#include <Editor/Domain/DomainFile.h>

namespace Horizon
{
	DomainFolder::DomainFolder(const DomainFolderDesc& desc, Engine* pEngine) : m_path(desc.folderPath),
		m_parent(desc.pParent), m_engine(pEngine)
	{
		auto* pAssetSub = m_engine->TryGetSubsystem<AssetSubsystem>();
		if (!pAssetSub)
		{
			Terminal::Error("DomainFile", "Could not catch asset subsystem");
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(m_path))
		{
			if (entry.is_directory())
			{
				DomainFolderDesc desc = {};
				desc.folderPath = entry.path();
				desc.pParent = this;
				auto* newFolder = Allocator::Create<DomainFolder>(CurrLoc(), desc, pEngine);
				m_subFolders.push_back(newFolder);
				continue;
			}

			if (entry.is_regular_file())
			{
				// TODO: Request metadata information from the file itself.
				// Fill the below.
				DomainFileDesc desc = {};
				desc.fileId = Guid::Generate(); // TODO: Not feasible if guid is already there.
				desc.pParent = this;
				desc.binaryPath = ""; // TODO: Fill this or find another way to handle this
				desc.metaPath = entry.path();

				auto* newFile = Allocator::Create<DomainFile>(CurrLoc(), desc, pEngine);
				m_files.push_back(newFile);
				continue;
			}
		}

		m_name = desc.folderPath.filename().string();
	}

	DomainFolder::~DomainFolder()
	{
		for (auto* file : m_files)
			Allocator::Delete(file);
		m_files.clear();

		for (auto* folder : m_subFolders)
			Allocator::Delete(folder);
		m_subFolders.clear();
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