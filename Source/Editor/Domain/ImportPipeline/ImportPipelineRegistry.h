#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionLibrary.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class ImportPipeline;

	class ImportPipelineRegistry
	{
		struct ImportPipelineEntry
		{
			ImportPipeline* pPipeline = nullptr;
			const Reflect::Type* pType = nullptr;
			List<std::string> extensions;
		};
	public:
		~ImportPipelineRegistry();

		void BootstrapViews(Engine::Engine* pEngine);

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library);
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library);

	private:
		b8 AddType(const Reflect::Type* pType);
		b8 RemoveType(const Reflect::Type* pType);

	private:
		Engine::Engine* m_engine = nullptr;
		List<ImportPipelineEntry> m_pipelines;
	};
}