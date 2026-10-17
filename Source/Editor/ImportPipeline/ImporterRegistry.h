#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionLibrary.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class ImportPipeline;
	class AssetImporter;

	struct ImportPipelineEntry
	{
		ImportPipeline* pPipeline = nullptr;
		Reflect::Type* pPipelineType = nullptr;
		Reflect::Type* pAssetType = nullptr;
		Reflect::Type* pSettingsType = nullptr;
		List<std::string> extensions;
	};

	struct AssetImporterEntry
	{
		Reflect::Type* pImportType = nullptr;
		Reflect::Type* pAssetType = nullptr;
		List<std::string> extensions;
	};

	class ImporterRegistry
	{
	public:
		~ImporterRegistry();

		void BootstrapViews(Engine::Engine* pEngine);

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library);
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library);

		ImportPipelineEntry* GetImportPipelineEntry(const std::string& extension);
		ImportPipelineEntry* GetImportPipelineEntry(Reflect::TypeHandle handle);
		AssetImporterEntry* GetAssetImporterEntry(Reflect::TypeHandle handle);

		AssetImporter* CreateImporterInstance(const std::string& ext);
		
	private:
		b8 AddPipelineType(const Reflect::Type* pType);
		b8 RemovePipelineType(const Reflect::Type* pType);

		b8 AddImporterType(const Reflect::Type* pType);
		b8 RemoveImporterType(const Reflect::Type* pType);

	private:
		Engine::Engine* m_engine = nullptr;
		Engine::ReflectionSystem* m_reflection = nullptr;

		List<ImportPipelineEntry> m_pipelines;
		std::unordered_map<std::string, usize> m_extToPipelineLookup;
		std::unordered_map<Reflect::TypeHandle, usize> m_typeToPipelineLookup;

		List<AssetImporterEntry> m_importers;
		std::unordered_map<Reflect::TypeHandle, usize> m_typeToImporterLookup;
		std::unordered_map<std::string, usize> m_extToImporterLookup;
	};
}