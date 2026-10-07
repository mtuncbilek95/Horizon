#pragma once

#include <Editor/Importer/AssetImporter.h>
#include <Engine/Core/Context.h>

namespace Horizon::Editor
{
	class EDITOR_API ImporterContext : public Engine::Context
	{
		struct ImporterEntry
		{
			const Reflect::Type* pType = nullptr;
			AssetImporter* pImporter = nullptr;
		};
	public:
		ImporterContext() = default;
		~ImporterContext() = default;

		Engine::ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

		AssetImporter* GetImporter(Reflect::TypeHandle handl) const;
		AssetImporter* GetImporter(const std::string& extension) const;

	private:
		b8 AddType(const Reflect::Type* pType);
		void RemoveType(const Reflect::Type* pType);

	private:
		Engine::ReflectionSystem* m_reflection = nullptr;

		List<ImporterEntry> m_importers;
		std::unordered_map<Reflect::TypeHandle, AssetImporter*> m_typeLookup;
		std::unordered_map<std::string, AssetImporter*> m_extLookup;
	};
}
