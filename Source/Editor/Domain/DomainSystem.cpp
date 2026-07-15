#include "DomainSystem.h"

#include <Editor/Project/ProjectContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/Importer/AssetImporterAttribute.h>

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
		auto& projSub = m_engine->GetContext<ProjectContext>();

		for (auto* manifest : m_importerManifest)
		{
			auto* attribute = manifest->GetCustomAttribute<AssetImporterAttribute>();
			std::vector<std::string_view> extensions = attribute->GetAssetExtension();
			auto it = std::find(extensions.begin(), extensions.end(), fileTypeExt);

			if (it != extensions.end())
				Terminal::Debug(GetName(), "{} has been found", fileTypeExt);
		}

		/*const ImporterTypeInfo* pInfo = ImporterRegistry::Get().Find(fileTypeExt);
		if (!pInfo)
		{
			Terminal::Warn("DomainSystem", "No importer registered for '{}'", fileTypeExt);
			return;
		}

		IAssetImporter* pImporter = pInfo->CreateImporter();

		Guid newId = Guid::Generate();
		std::filesystem::path targetMetaPath = targetFolder->GetAbsolutePath() / (std::string(pInfo->defaultName) + ".hmeta");
		std::filesystem::path targetCookPath = projSub.GetCookedPath() / (newId.ToString() + fileTypeExt);

		AssetImportContext context({ targetMetaPath, targetCookPath }, newId);

		// TODO: TEMPORARY SOLUTION
		if (!PAL::File::Create(targetMetaPath))
		{
			Terminal::Error(GetName(), "Failed to create meta {}", targetMetaPath.string());
			return;
		}

		if (!PAL::File::Create(targetCookPath))
		{
			Terminal::Error(GetName(), "Failed to create cooked {}", targetCookPath.string());
			PAL::File::Delete(targetMetaPath);
			return;
		}

		// This mf will fill the .hmeta.
		pImporter->OnImportDefault(context);

		Allocator::Delete(pImporter);*/
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
					// Read with a serializer or read json here with nlohmann.

					DomainFileDesc desc = {};
					desc.fileId = Guid::Generate();
					desc.fileName = entry.path().stem().string();
					desc.pParent = pTarget;
					pFile = Allocator::Create<DomainFile>(CurrLoc(), desc, m_engine);
					pTarget->AddFile(pFile);

					Terminal::Info("DomainSystem", "{} file has been added as {}", pFile->GetName(), pFile->GetMetaPath().string());
				}
				pFile->Mark();
			}
		}

		pTarget->SweepUnmarked();

		for (auto* sub : pTarget->GetSubfolders())
			UpdateFolder(sub);
	}
}