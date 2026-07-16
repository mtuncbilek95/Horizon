#include "DomainSystem.h"

#include <Editor/Project/ProjectContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/Importer/AssetImporterAttribute.h>
#include <Editor/Domain/Importer/AssetImportContext.h>
#include <Editor/Domain/Importer/IAssetImporter.h>

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

		m_rootPath = pProjectSub->GetDomainPath();
		if (!std::filesystem::exists(m_rootPath))
			std::filesystem::create_directory(m_rootPath);

		DomainFolderDesc rootDesc = {};
		rootDesc.absolutePath = m_rootPath;
		rootDesc.relativePath = m_rootPath.filename();
		rootDesc.folderName = m_rootPath.filename().string();
		rootDesc.pParent = nullptr;
		m_rootFolder = Allocator::Create<DomainFolder>(CurrLoc(), rootDesc, m_engine);

		if (!m_rootFolder)
			return EngineReport("Could not create root domain folder.");

		auto* manifestCtx = pEngine->GetModuleContext();
		m_importerManifest = manifestCtx->GetManifestsByBase(TypeIdOf<IAssetImporter>());

		Terminal::Debug(GetName(), "{} importers has been registered to domain system", m_importerManifest.size());

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

	void DomainSystem::AddNewFolder(DomainFolder* targetFolder)
	{
		if (!targetFolder)
		{
			Terminal::Warn(GetName(), "target folder is a valid folder in vfs");
			return;
		}

		// Some magic regex stuff (WTF?)
		const std::regex reg(R"(^New\s*Folder(?:\s*\((\d+)\))?$)", std::regex::icase);

		i64 nameCounter = -1;
		for (const auto& entry : std::filesystem::directory_iterator(targetFolder->GetAbsolutePath()))
		{
			if (!entry.is_directory())
				continue;

			std::smatch newMatch;

			const std::string name = entry.path().filename().string();
			if (std::regex_match(name, newMatch, reg))
			{
				i64 n = newMatch[1].matched ? std::stoi(newMatch[1].str()) : 0;
				nameCounter = std::max(nameCounter, n);
			}
		}

		std::filesystem::path newPath = targetFolder->GetAbsolutePath();
		if (nameCounter < 0)
			newPath /= "New Folder";
		else
			newPath /= "New Folder (" + std::to_string(nameCounter + 1) + ")";

		std::filesystem::create_directory(newPath);
	}

	void DomainSystem::ImportDefault(DomainFolder* targetFolder, const std::string& fileTypeExt)
	{
		if (!targetFolder)
		{
			Terminal::Warn(GetName(), "Invalid target folder");
			return;
		}
		auto& projSub = m_engine->GetContext<ProjectContext>();

		TypeManifest* foundManifest = nullptr;
		AssetImporterAttribute* foundAttr = nullptr;

		for (TypeManifest* manifest : m_importerManifest)
		{
			AssetImporterAttribute* attr = manifest->GetCustomAttribute<AssetImporterAttribute>();
			if (!attr)
				continue;

			const auto& exts = attr->GetAssetExtension();
			if (std::find(exts.begin(), exts.end(), fileTypeExt) != exts.end())
			{
				foundManifest = manifest;
				foundAttr = attr;
				break;
			}
		}

		if (!foundManifest)
		{
			Terminal::Warn(GetName(), "No importer for '{}'", fileTypeExt);
			return;
		}

		Guid newId = Guid::Generate();
		std::filesystem::path metaPath = targetFolder->GetAbsolutePath() /
			(std::string(foundAttr->GetDefaultName()) + ".hmeta");
		std::filesystem::path cookPath = projSub.GetCookedPath() / (newId.ToString() + fileTypeExt);

		if (!PAL::File::Create(metaPath))
		{
			Terminal::Error(GetName(), "Failed to create meta {}", metaPath.string());
			return;
		}

		if (!PAL::File::Create(cookPath))
		{
			Terminal::Error(GetName(), "Failed to create cooked {}", cookPath.string());
			PAL::File::Delete(metaPath);
			return;
		}

		IAssetImporter* importer = static_cast<IAssetImporter*>(foundManifest->Create());
		if (!importer)
		{
			Terminal::Error(GetName(), "manifest->Create() returned null for importer");
			return;
		}

		AssetImportContext context({ metaPath, cookPath }, newId);
		importer->OnImportDefault(context);

		Allocator::Delete(importer);

		if (auto* assetSub = m_engine->TryGetSystem<AssetSystem>())
			assetSub->RegisterAsset(newId, cookPath);

		Terminal::Log(GetName(), "Created '{}' (guid {})", metaPath.filename().string(), newId.ToString());
	}

	void DomainSystem::UpdateFolder(DomainFolder* pTarget)
	{
		pTarget->ResetChildMarks();

		for (const auto& entry : std::filesystem::directory_iterator(pTarget->GetAbsolutePath()))
		{
			if (entry.is_directory())
			{
				DomainFolder* pFolder = pTarget->FindFolder(entry.path().filename().string());
				if (!pFolder)
				{
					DomainFolderDesc desc = {};
					desc.absolutePath = entry.path();
					desc.relativePath = pTarget->GetRelativePath() / entry.path().filename();
					desc.folderName = entry.path().filename().string();
					desc.pParent = pTarget;
					pFolder = Allocator::Create<DomainFolder>(CurrLoc(), desc, m_engine);
					pTarget->AddSubfolder(pFolder);

					Terminal::Info("DomainSystem", "{} folder has been added as {}", pFolder->GetName(), pFolder->GetRelativePath());
				}
				pFolder->Mark();
			}
			else if (entry.is_regular_file())
			{
				if (entry.path().extension() != ".hmeta")
					continue;

				DomainFile* pFile = pTarget->FindFile(entry.path().stem().string());
				if (!pFile)
				{
					DomainFileDesc desc = {};
					desc.pParent = pTarget;
					desc.metaPath = entry.path();

					pFile = Allocator::Create<DomainFile>(CurrLoc(), desc, m_engine);

					if (!pFile->IsValid())
					{
						Terminal::Warn("DomainSystem", "Invalid meta, skipping: {}", entry.path().string());
						Allocator::Delete(pFile);
						continue;
					}

					pTarget->AddFile(pFile);
					Terminal::Info("DomainSystem", "{} added (guid {})", pFile->GetName(), pFile->GetGuid().ToString());
				}
				pFile->Mark();
			}
		}

		pTarget->SweepUnmarked();

		for (auto* sub : pTarget->GetSubfolders())
			UpdateFolder(sub);
	}
}