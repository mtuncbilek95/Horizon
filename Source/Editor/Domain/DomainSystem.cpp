#include "DomainSystem.h"

#include <Editor/Project/ProjectContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/ImportPipeline/ImportFileSettings.h>
#include <Editor/Domain/ImportPipeline/ImportFileAttribute.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>
#include <Engine/Asset/AssetSystem.h>

#include <Runtime/Serialization/JsonArchive.h>
#include <Runtime/Serialization/Serializer.h>
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

		// Get all the FileImportSettings from reflection.
		auto* moduleCtx = pEngine->GetModuleContext();

		for (Reflect::Type* settingsType : moduleCtx->GetTypeByAttribute(Reflect::TypeOf<ImportFileAttribute>()))
		{
			ImportFileAttribute* attr = settingsType->GetCustomAttribute<ImportFileAttribute>();
			if (!attr)
				continue;

			auto* settings = static_cast<ImportFileSettings*>(settingsType->CreateFromMemory());
			if (!settings)
			{
				Terminal::Warn(GetName(), "CreateFromMemory returned null for import settings");
				continue;
			}

			m_importSettings.emplace(attr->GetTypeHandle(), settings);
		}

		return EngineReport();
	}

	void DomainSystem::OnSync()
	{
		UpdateFolder(m_rootFolder);
	}

	void DomainSystem::OnDetach()
	{
		Allocator::Delete(m_rootFolder);

		for (auto& [handle, settings] : m_importSettings)
			Allocator::Delete(settings);

		m_importSettings.clear();
	}

	void DomainSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<AssetSystem>(rules.after);
	}

	void DomainSystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<AssetSystem>(rules.after);
	}

	void DomainSystem::ImportDefault(DomainFolder* targetFolder, const ImportDescriptor& descriptor)
	{
		// Check target
		if (!targetFolder)
		{
			Terminal::Warn(GetName(), "Invalid target folder");
			return;
		}

		// Check fileType
		if (!descriptor.fileType)
		{
			Terminal::Warn(GetName(), "Descriptor has no file type");
			return;
		}

		// Get proper importSettings
		auto it = m_importSettings.find(descriptor.fileType->GetTypeId());
		if (it == m_importSettings.end())
		{
			Terminal::Warn(GetName(), "No import settings for '{}'", descriptor.fileType->GetName());
			return;
		}

		// Write on hmeta
		MetaHeader header = {};
		it->second->OnImportDefault(header);

		auto* moduleCtx = m_engine->GetModuleContext();
		const Reflect::Type* headerType = moduleCtx->GetType(Reflect::TypeOf<MetaHeader>());
		if (!headerType)
		{
			Terminal::Error(GetName(), "MetaHeader is not registered");
			return;
		}

		Serializer serializer(moduleCtx,
			[](void* ud, Reflect::TypeHandle h) -> const Reflect::Type*
			{
				return static_cast<ModuleContext*>(ud)->GetType(h);
			});

		// Serialize it
		JsonArchiveWriter writer;
		serializer.Serialize(&header, *headerType, writer);

		// Create it
		std::filesystem::path metaPath = targetFolder->GetFolderPath() / (descriptor.fileName + ".hmeta");
		if (!PAL::File::Create(metaPath))
			Terminal::Error(GetName(), "Failed to create meta {}", metaPath.string());
		
		// Write on it
		PAL::FileAccessRequest requestMeta = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);
		if (!PAL::File::WriteString(requestMeta, writer.ToString()))
			Terminal::Error(GetName(), "Failed to write meta {}", metaPath.string());
		PAL::File::ReleaseAccess(requestMeta);

		// TODO: Create .hcooked + Register it via AssetSystem.
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

			if (pTarget->HasFile(entry.path().filename().string()))
				continue;

			DomainFileDesc fileDesc = {};
			fileDesc.name = entry.path().filename().string();
			fileDesc.metaPath = entry.path();
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
		if (!std::filesystem::exists(pTarget->GetMetaPath()))
			Allocator::Delete(pTarget);

		// If corrupt file, we're doomed.
	}
}