#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class AssetLoadStrategy;

	class H_EXPORT AssetService : public Service
	{
	public:
		AssetService() = default;
		~AssetService() = default;

		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		AssetLoadStrategy* FindStrategy(Reflect::TypeHandle assetType);

	private:
		List<AssetLoadStrategy*> m_loaders;
		std::unordered_map<Reflect::TypeHandle, usize> m_loaderLookup;
	};
}