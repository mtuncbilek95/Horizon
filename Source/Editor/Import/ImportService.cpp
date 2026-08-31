#include "ImportService.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Import/ImportTypeAttribute.h>

#include <Engine/Content/ContentContext.h>
#include <Engine/Content/ContentMount.h>
#include <Engine/Content/ContentFileWriter.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <utility>

namespace Horizon::Editor
{
	namespace
	{
		class MountImportSink final : public ImportSink
		{
		public:
			MountImportSink(DomainFile* pFile, Engine::ContentMount* pOutput) : m_file(pFile), m_output(pOutput)
			{
			}

			Guid ResolveSubAsset(std::string_view name, std::string_view assetTypeName) final
			{
				for (const DomainSubAsset& subAsset : m_file->GetMeta().subAssets)
				{
					if (subAsset.name != name)
						continue;

					return subAsset.id;
				}

				DomainSubAsset subAsset;
				subAsset.name = std::string(name);
				subAsset.assetTypeName = std::string(assetTypeName);
				subAsset.id = Guid::Generate();

				DomainMeta meta = m_file->GetMeta();
				meta.subAssets.PushBack(subAsset);

				m_file->WriteMeta(meta);

				return subAsset.id;
			}

			Engine::ContentFileWriter* Open(const Guid& guid, std::string_view assetTypeName) final
			{
				return Memory::Allocator::Create<Engine::ContentFileWriter>(Memory::CurrLoc(), guid, assetTypeName);
			}

			void Close(Engine::ContentFileWriter* pWriter) final
			{
				List<u8> bytes;

				if (pWriter->Build(bytes))
					m_output->Write(pWriter->GetID(), bytes);

				Memory::Allocator::Delete(pWriter);
			}

		private:
			DomainFile* m_file = nullptr;
			Engine::ContentMount* m_output = nullptr;
		};
	}

	Engine::ModuleReport ImportService::OnInitialize()
	{
		m_clock.Start();

		// Get domain service
		m_domain = GetEngine()->RequestService<DomainService>();
		if (!m_domain)
			return Engine::ModuleReport("Domain service is not accessible. Either a creation problem or dependency problem.");

		// Get content context
		m_content = GetEngine()->RequestContext<Engine::ContentContext>();
		if (!m_content)
			return Engine::ModuleReport("Content context is not accessible. Either a creation problem or dependency problem.");

		// Since this service is for editor, get Cooked folder that runs on the project.
		m_output = m_content->FindMutableMount();
		if (m_output == nullptr)
			return Engine::ModuleReport("Import service has no writable mount");

		// Register importers via runtime reflection
		RegisterImporters();

		// Handle onAdded + onModified events for watcher's reaction
		BindWatcher();

		// First check
		SweepFolder(m_domain->GetRoot());

		return Engine::ModuleReport();
	}

	void ImportService::OnExecute()
	{
		// Get time to not create a shit show
		const f64 now = m_clock.GetElapsedTimeInSec();
		usize budget = MaxImportsPerFrame;

		for (usize i = 0; i < m_pending.GetCount() && budget > 0;)
		{
			if (now - m_pending[i].timestamp < DebounceSeconds)
			{
				++i;
				continue;
			}

			const std::string relativePath = m_pending[i].relativePath;

			m_pending.RemoveAt(i);
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
		// Resolve files
		for (DomainFile* pFile : pFolder->GetFiles())
			Evaluate(pFile);

		// check the sub folders
		for (DomainFolder* pChild : pFolder->GetFolders())
			SweepFolder(pChild);
	}

	b8 ImportService::PrepareMeta(DomainFile* pFile)
	{
		List<const ImporterEntry*> candidates;
		CollectCandidates(StringOps::OnlyExtension(pFile->GetName()), candidates);

		if (candidates.IsEmpty())
			return false;

		if (!pFile->LoadMeta())
		{
			DomainMeta meta;
			meta.id = Guid::Generate();
			meta.assetTypeName = candidates.Front()->assetTypeName;

			return pFile->WriteMeta(meta);
		}

		if (!pFile->GetID().IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} carries a broken meta", pFile->GetRelativePath());
			return false;
		}

		if (!pFile->GetMeta().assetTypeName.empty())
			return true;

		DomainMeta meta = pFile->GetMeta();
		meta.assetTypeName = candidates.Front()->assetTypeName;

		return pFile->WriteMeta(meta);
	}

	void ImportService::Process(DomainFile* pFile)
	{
		if (!PrepareMeta(pFile))
			return;

		AssetImporter* pImporter = FindImporter(pFile->GetMeta().assetTypeName);

		if (pImporter == nullptr)
			return;

		ImportRequest request;
		request.guid = pFile->GetID();
		request.sourcePath = pFile->GetSourcePath();
		request.extension = StringOps::OnlyExtension(pFile->GetName());
		request.assetTypeName = pFile->GetMeta().assetTypeName;
		request.pReflection = GetEngine()->GetReflectionSystem();

		MountImportSink sink(pFile, m_output);

		if (!pImporter->Import(request, sink))
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be imported", pFile->GetRelativePath());
			return;
		}

		Terminal::Info(StringOps::GetName(this), "{} has been imported as {}", pFile->GetRelativePath(),
			request.assetTypeName);
	}

	void ImportService::Evaluate(DomainFile* pFile)
	{
		if (!PrepareMeta(pFile))
			return;

		if (m_output->Contains(pFile->GetID()))
			return;

		Enqueue(pFile->GetRelativePath());
	}

	void ImportService::Enqueue(std::string_view relativePath)
	{
		if (relativePath.ends_with(DomainFile::MetaSuffix))
			return;

		const f64 now = m_clock.GetElapsedTimeInSec();

		for (PendingImport& pending : m_pending)
		{
			if (pending.relativePath != relativePath)
				continue;

			pending.timestamp = now;
			return;
		}

		PendingImport pending;
		pending.relativePath = std::string(relativePath);
		pending.timestamp = now;

		m_pending.PushBack(std::move(pending));
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