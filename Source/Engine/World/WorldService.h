#pragma once

#include <Engine/Core/Service.h>

namespace Horizon::Engine
{
	class H_EXPORT WorldService : public Service
	{
	public:
		WorldService() = default;
		~WorldService() = default;

		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

	private:

	private:
	};
}