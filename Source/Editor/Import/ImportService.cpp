#include "ImportService.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Import/ImportTypeAttribute.h>

#include <Engine/Content/ContentContext.h>
#include <Engine/Content/ContentMount.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <utility>

namespace Horizon::Editor
{
	Engine::ModuleReport ImportService::OnInitialize()
	{
		m_domain = GetEngine()->RequestService<DomainService>();
		m_content = GetEngine()->RequestContext<Engine::ContentContext>();

		if (m_domain == nullptr || m_content == nullptr)
			return Engine::ModuleReport("Import service has no domain or content layer");

		m_output = m_content->FindMutableMount();

		if (m_output == nullptr)
			return Engine::ModuleReport("Import service has no writable mount");

		RegisterImporters();
		BindWatcher();
		SweepFolder(m_domain->GetRoot());

		return Engine::ModuleReport();
	}

	void ImportService::OnExecute()
	{
		usize budget = MaxImportsPerFrame;

		while (!m_pending.IsEmpty() && budget > 0)
		{
			const std::string relativePath = m_pending.Front();

			m_pending.PopFront();
			--budget;

			DomainFile* pFile = ResolveFile(relativePath);

			if (pFile == nullptr)
				continue;

			Process(pFile);
		}
	}

	void ImportService::OnFinalize()
	{
		for (ImporterEntry& entry : m_importers)
			Memory::Allocator::Delete(entry.pImporter);

		m_importers.Clear();
		m_pending.Clear();

		m_output = nullptr;
		m_content = nullptr;
		m_domain = nullptr;
	}

	void ImportService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Requires<DomainService>();
		graph.Requires<Engine::ContentContext>();
	}

	void ImportService::RegisterImporters()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();

		List<Reflect::Type*> pTypes = pReflect->GetTypeByAttribute(Reflect::TypeOf<ImportTypeAttribute>());

		for (auto* pType : pTypes)
		{
			if (pType->GetBaseId() != Reflect::TypeOf<AssetImporter>())
			{
				Terminal::Error(StringOps::GetName(this), "{} doesn't have AssetImporter as its inheritance.", pType->GetName());
				continue;
			}

			ImportTypeAttribute* pAttribute = pType->GetCustomAttribute<ImportTypeAttribute>();

			if (pAttribute == nullptr)
			{
				Terminal::Error(StringOps::GetName(this), "{} carries no import type attribute", pType->GetName());
				continue;
			}

			Reflect::Type* pAssetType = pReflect->GetType(pAttribute->GetAssetHandle());

			if (pAssetType == nullptr)
			{
				Terminal::Error(StringOps::GetName(this), "{} targets an unregistered asset type", pType->GetName());
				continue;
			}

			Terminal::Info(StringOps::GetName(this), "{} has been registered for {}", pType->GetName(), pAssetType->GetName());

			ImporterEntry entry;
			entry.pImporter = (AssetImporter*)pType->Create();
			entry.assetHandle = pAttribute->GetAssetHandle();
			entry.assetTypeName = pAssetType->GetName();
			entry.extensions = pAttribute->GetExtensions();

			m_importers.PushBack(std::move(entry));
		}
	}

	void ImportService::BindWatcher()
	{
		PAL::DirectoryWatcher& watcher = m_domain->GetWatcher();

		watcher.OnAdded([this](const PAL::DirectoryWatcher::Event& event)
			{
				Enqueue(event.relativePath);
			});

		watcher.OnModified([this](const PAL::DirectoryWatcher::Event& event)
			{
				Enqueue(event.relativePath);
			});
	}

	AssetImporter* ImportService::FindImporter(std::string_view assetTypeName) const
	{
		for (const ImporterEntry& entry : m_importers)
		{
			if (StringOps::EqualsNoCase(entry.assetTypeName, assetTypeName))
				return entry.pImporter;
		}

		Terminal::Error(StringOps::GetName(this), "{} asset type has no importer", assetTypeName);
		return nullptr;
	}

	void ImportService::CollectCandidates(std::string_view extension, List<const ImporterEntry*>& outCandidates) const
	{
		for (const ImporterEntry& entry : m_importers)
		{
			for (const std::string& candidate : entry.extensions)
			{
				if (!StringOps::EqualsNoCase(candidate, extension))
					continue;

				outCandidates.PushBack(&entry);
				break;
			}
		}
	}

	void ImportService::SweepFolder(DomainFolder* pFolder)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
			Evaluate(pFile);

		for (DomainFolder* pChild : pFolder->GetFolders())
			SweepFolder(pChild);
	}

	b8 ImportService::PrepareMeta(DomainFile* pFile)
	{
		List<const ImporterEntry*> candidates;
		CollectCandidates(StringOps::OnlyExtension(pFile->GetName()), candidates);

		if (candidates.IsEmpty())
			return false;

		if (!pFile->EnsureMeta())
			return false;

		if (!pFile->GetMeta().assetTypeName.empty())
			return true;

		return pFile->SetAssetType(candidates.Front()->assetTypeName);
	}

	void ImportService::Evaluate(DomainFile* pFile)
	{
		if (!PrepareMeta(pFile))
			return;

		if (m_output->Contains(pFile->GetID()))
			return;

		Enqueue(pFile->GetRelativePath());
	}

	void ImportService::Process(DomainFile* pFile)
	{
		if (!pFile->ReloadMeta() || !PrepareMeta(pFile))
			return;

		AssetImporter* pImporter = FindImporter(pFile->GetMeta().assetTypeName);

		if (pImporter == nullptr)
			return;

		Terminal::Info(StringOps::GetName(this), "{} is ready to be imported as {}", pFile->GetRelativePath(),
			pFile->GetMeta().assetTypeName);
	}

	void ImportService::Enqueue(std::string_view relativePath)
	{
		std::string source(relativePath);

		if (source.ends_with(DomainFile::MetaSuffix))
			source.resize(source.size() - DomainFile::MetaSuffix.size());

		if (m_pending.Contains(source))
			return;

		m_pending.PushBack(std::move(source));
	}

	DomainFile* ImportService::ResolveFile(const std::string& relativePath) const
	{
		const usize separator = relativePath.find_last_of('/');

		const std::string_view parentPath = separator == std::string::npos ?
			std::string_view() : std::string_view(relativePath).substr(0, separator);

		const std::string_view name = separator == std::string::npos ?
			std::string_view(relativePath) : std::string_view(relativePath).substr(separator + 1);

		DomainFolder* pFolder = m_domain->FindFolder(parentPath);

		if (pFolder == nullptr)
			return nullptr;

		return pFolder->FindFile(name);
	}
}