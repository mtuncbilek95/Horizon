#pragma once

#include <Engine/Core/Service.h>

namespace Horizon::Engine
{
	class H_EXPORT AssetService : public Service
	{
	public:
		AssetService() = default;
		~AssetService() = default;

		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

	private:

	private:
	};
}