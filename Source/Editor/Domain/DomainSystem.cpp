#include "DomainSystem.h"

#include <Editor/Project/ProjectContext.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>
#include <Engine/Asset/AssetSystem.h>

#include <Runtime/PAL/File/File.h>

#include <regex>
#include <algorithm>
#include <string>

namespace Horizon
{
	EngineReport DomainSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		auto* pProjectSub = pEngine->TryGetContext<ProjectContext>();
		if (!pProjectSub)
			return EngineReport("There is no Project Context");

		// Get root path and check if its created in Project Context.
		const auto& projectPath = pProjectSub->GetProjectFolderPath();

		const auto& assetFolderPath = pProjectSub->GetDomainPath();
		if (!std::filesystem::exists(assetFolderPath))
			std::filesystem::create_directory(assetFolderPath);

		// Create root folder as virtual.
		DomainFolderDesc rootDesc = {};
		rootDesc.folderPath = assetFolderPath;
		rootDesc.relativePath = std::filesystem::relative(assetFolderPath, projectPath);
		rootDesc.folderName = assetFolderPath.filename().string();
		rootDesc.parentFolder = nullptr;
		m_rootFolder = Allocator::Create<DomainFolder>(CurrLoc(), rootDesc, m_engine);

		if (!m_rootFolder)
			return EngineReport("Could not create root domain folder.");
		UpdateFolder(m_rootFolder);

		// Get all the IAssetImporters from reflection.
		auto* pModCtx = pEngine->GetModuleContext();

		return EngineReport();
	}

	void DomainSystem::OnSync()
	{
		UpdateFolder(m_rootFolder);
	}

	void DomainSystem::OnDetach()
	{
		Allocator::Delete(m_rootFolder);
	}

	void DomainSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<AssetSystem>(rules.after);
	}

	void DomainSystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<AssetSystem>(rules.after);
	}

	void DomainSystem::ImportDefault(DomainFolder* targetFolder, const ImportDescriptor& importInfo)
	{
		// Check if the targeted folder is valid
		if (!targetFolder)
		{
			Terminal::Warn(GetName(), "Invalid target folder");
			return;
		}
	}

	void DomainSystem::UpdateFolder(DomainFolder* pTarget)
	{
		// Check if its valid in OS side
		if (!std::filesystem::exists(pTarget->GetFolderPath()))
		{
			Allocator::Delete(pTarget);
			return;
		}

		// Add if there is a new child folder
		for (const auto& entry : std::filesystem::directory_iterator(pTarget->GetFolderPath()))
		{
			if (!entry.is_directory())
				continue;

			if (pTarget->HasFolder(entry.path().filename().string()))
				continue;

			DomainFolderDesc folderDesc = {};
			folderDesc.folderPath = entry.path();
			folderDesc.relativePath = pTarget->GetRelativePath() / entry.path().filename().string();
			folderDesc.folderName = entry.path().filename().string();
			folderDesc.parentFolder = pTarget;
			pTarget->m_subFolders.push_back(Allocator::Create<DomainFolder>(CurrLoc(), folderDesc, m_engine));
		}

		// Check folders one by one.
		for (auto* pNewTarget : pTarget->GetSubFolders())
			UpdateFolder(pNewTarget);

		// Add if there is a new child folder
		for (const auto& entry : std::filesystem::directory_iterator(pTarget->GetFolderPath()))
		{
			if (entry.is_directory() || entry.path().extension() != ".hmeta")
				continue;

			DomainFileDesc fileDesc = {};
			fileDesc.name = entry.path().filename().string();
			fileDesc.assetType = nullptr;
			fileDesc.assetTypeName = "Unknown";
			fileDesc.parentFolder = pTarget;
			pTarget->m_files.push_back(Allocator::Create<DomainFile>(CurrLoc(), fileDesc, m_engine));
		}

		// Check files one by one
		for (auto* pFile : pTarget->GetFiles())
			UpdateFile(pFile);
	}

	void DomainSystem::UpdateFile(DomainFile* pTarget)
	{
		// Check if meta or cook is missing
		if (!std::filesystem::exists(pTarget->GetMetaPath()) || !std::filesystem::exists(pTarget->GetCookedPath()))
			Allocator::Delete(pTarget);

		// If corrupt file, we're doomed.
	}
}