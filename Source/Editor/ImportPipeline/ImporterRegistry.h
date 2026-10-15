#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionLibrary.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class ImportPipeline;
	class AssetImporter;

	class ImporterRegistry
	{
		struct PipelineEntry
		{
			ImportPipeline* pPipeline = nullptr;
			const Reflect::Type* pType = nullptr;
			List<std::string> extensions;
		};

		struct ImporterEntry
		{
			AssetImporter* pImporter = nullptr;
			const Reflect::Type* pType = nullptr;
		};

	public:
		~ImporterRegistry();

		void BootstrapViews(Engine::Engine* pEngine);

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library);
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library);

		template<typename T>
		T* GetPipeline()
		{
			return (T*)GetPipeline(Reflect::TypeOf<T>());
		}
		ImportPipeline* GetPipeline(Reflect::TypeHandle handl);
		ImportPipeline* GetPipeline(const std::string extension);

	private:
		b8 AddPipelineType(const Reflect::Type* pType);
		b8 RemovePipelineType(const Reflect::Type* pType);

		b8 AddImporterType(const Reflect::Type* pType);
		b8 RemoveImporterType(const Reflect::Type* pType);

	private:
		Engine::Engine* m_engine = nullptr;
		List<PipelineEntry> m_pipelines;
		std::unordered_map<std::string, usize> m_pipelineLookup;
		std::unordered_map<Reflect::TypeHandle, usize> m_typeToPipelineLookup;

		List<ImporterEntry> m_importers;
		std::unordered_map<Reflect::TypeHandle, usize> m_typeToImporterLookup;
	};
}