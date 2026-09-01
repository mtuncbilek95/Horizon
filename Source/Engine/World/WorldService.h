#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Engine
{
	class System;

	class H_EXPORT WorldService : public Service
	{
	public:
		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

	private:
		List<System*> m_systems;
	};
}