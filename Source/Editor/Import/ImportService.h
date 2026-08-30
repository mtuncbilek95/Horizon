#pragma once

#include <Editor/Import/AssetImporter.h>
#include <Engine/Core/Service.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Engine
{
	class ContentContext;
	class ContentMount;
}

namespace Horizon::Editor
{
	class DomainService;
	class DomainFolder;
	class DomainFile;

	class H_EXPORT ImportService : public Engine::Service
	{
		static constexpr usize MaxImportsPerFrame = 4;

		struct ImporterEntry
		{
			AssetImporter* pImporter = nullptr;
			Reflect::TypeHandle assetHandle;
			std::string assetTypeName;
			List<std::string> extensions;
		};

	public:
		ImportService() = default;
		~ImportService() = default;

		Engine::ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;

		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		AssetImporter* FindImporter(std::string_view assetTypeName) const;

	private:
		void RegisterImporters();
		void BindWatcher();

		void CollectCandidates(std::string_view extension, List<const ImporterEntry*>& outCandidates) const;

		void SweepFolder(DomainFolder* pFolder);
		b8 PrepareMeta(DomainFile* pFile);
		void Evaluate(DomainFile* pFile);
		void Process(DomainFile* pFile);

		void Enqueue(std::string_view relativePath);
		DomainFile* ResolveFile(const std::string& relativePath) const;

		DomainService* m_domain = nullptr;
		Engine::ContentContext* m_content = nullptr;
		Engine::ContentMount* m_output = nullptr;

		List<ImporterEntry> m_importers;
		List<std::string> m_pending;
	};
}