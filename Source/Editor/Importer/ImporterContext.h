#pragma once

#include <Editor/Importer/AssetImporter.h>
#include <Engine/Core/Context.h>

namespace Horizon::Editor
{
	class H_EXPORT ImporterContext : public Engine::Context
	{
	public:
		ImporterContext() = default;
		~ImporterContext() = default;

		Engine::ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		AssetImporter* GetImporter(Reflect::TypeHandle handl) const;
		AssetImporter* GetImporter(const std::string& extension) const;

	private:
		Engine::ReflectionSystem* m_reflection = nullptr;

		List<AssetImporter*> m_assets;
		std::unordered_map<Reflect::TypeHandle, usize> m_typeLookup;
		std::unordered_map<std::string, usize> m_extLookup;
	};
}