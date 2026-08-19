#pragma once

#include <Engine/Core/Service.h>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;

	class H_EXPORT DomainService : public Engine::Service
	{
	public:
		DomainService();
		~DomainService();

		Engine::ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;

		void DeclareDependencies(Engine::ModuleGraph& graph) final;
	};
}