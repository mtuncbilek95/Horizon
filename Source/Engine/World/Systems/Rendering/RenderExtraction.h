#pragma once

#include <Engine/Asset/Mesh/MeshUsePermit.h>
#include <Engine/World/System.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetService;
}

namespace Horizon::World::System
{
	HCLASS();
	class RenderExtraction : public Engine::System
	{
		HORIZON_TYPE_REFLECT(RenderExtraction);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;

	private:
		Engine::AssetService* m_assets = nullptr;

		List<Asset::MeshUsePermit> m_permits;
	};
}