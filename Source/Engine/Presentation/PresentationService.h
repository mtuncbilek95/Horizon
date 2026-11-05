#pragma once

#include <Engine/Core/Service.h>

namespace Horizon::Engine
{
	class ENGINE_API PresentationService : public Service 
	{
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;
	};
}