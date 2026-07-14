#include "DomainSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSystem.h>

#include <Editor/Project/ProjectContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/Importer/ImporterRegistry.h>
#include <Editor/Domain/Importer/IAssetImporter.h>
#include <Editor/Domain/Importer/AssetImportContext.h>

namespace Horizon
{
	EngineReport DomainSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		auto* pProjectSub = pEngine->TryGetContext<ProjectContext>();

		m_rootPath = pProjectSub->GetDomainPath();
		if (!std::filesystem::exists(m_rootPath))
			std::filesystem::create_directory(m_rootPath);

		DomainFolderDesc rootDesc = {};
		rootDesc.folderPath = m_rootPath;
		rootDesc.pParent = nullptr;
		m_rootFolder = Allocator::Create<DomainFolder>(CurrLoc(), rootDesc, m_engine);

#if defined(HORIZON_DEBUG)
		for (auto* file : m_rootFolder->GetFiles())
			Terminal::Log(m_rootFolder->GetName(), "{}", file->GetName());

		for (auto* folder : m_rootFolder->GetSubfolders())
		{
			Terminal::Log(m_rootFolder->GetName(), "{}", folder->GetName());
			RecursiveDebugChecker(folder);
		}
#endif

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

	void DomainSystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<AssetSystem>(rules.after);
	}

	void DomainSystem::ImportDefault(const std::filesystem::path& source)
	{
		std::string ext = source.extension().string();

		const ImporterTypeInfo* pInfo = ImporterRegistry::Get().Find(ext);
		if (!pInfo)
		{
			Terminal::Warn("DomainSystem", "No importer registered for '{}'", ext);
			return;
		}

		IAssetImporter* pImporter = pInfo->CreateImporter();

		AssetImportContext context(source, Guid::Generate());
		pImporter->OnImportDefault(context);

		Terminal::Log("DomainSystem", "Default-imported '{}' -> '{}'",
			source.string(), context.BinaryPath().string());

		Allocator::Delete(pImporter);
	}

	void DomainSystem::RecursiveDebugChecker(DomainFolder* folder)
	{
		for (auto* file : folder->GetFiles())
			Terminal::Log(folder->GetName(), "{}", file->GetName());

		for (auto* fd : folder->GetSubfolders())
			RecursiveDebugChecker(fd);
	}

	void DomainSystem::UpdateFolder(DomainFolder* pTarget)
	{
		pTarget->ResetChildMarks();

		for (const auto& entry : std::filesystem::directory_iterator(pTarget->GetPath()))
		{
			if (entry.is_directory())
			{
				DomainFolder* pFolder = pTarget->FindFolder(entry.path().filename().string());
				if (!pFolder)
				{
					DomainFolderDesc desc = {};
					desc.folderPath = entry.path();
					desc.pParent = pTarget;
					pFolder = Allocator::Create<DomainFolder>(CurrLoc(), desc, m_engine);
					pTarget->AddSubfolder(pFolder);

					Terminal::Info("DomainSystem", "{} folder has been added as {}", pFolder->GetName(), pFolder->GetPath().string());
				}
				pFolder->Mark();
			}
			else if (entry.is_regular_file())
			{
				DomainFile* pFile = pTarget->FindFile(entry.path().filename().string());
				if (!pFile)
				{
					DomainFileDesc desc = {};
					desc.fileId = Guid::Generate();
					desc.pParent = pTarget;
					desc.metaPath = entry.path();
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