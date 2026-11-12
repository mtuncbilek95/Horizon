#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Material/Material.h>
#include <Engine/Asset/Material/MaterialUsePermit.h>
#include <Engine/Asset/Mesh/MeshUsePermit.h>
#include <Engine/World/System.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetService;
}

namespace Horizon::Asset
{
	class MaterialStreamer;
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
		u32 ResolveMaterial(Engine::AssetHandle<Asset::Material>& handle);

	private:
		Engine::AssetService* m_assets = nullptr;
		Asset::MaterialStreamer* m_materials = nullptr;

		List<Asset::MeshUsePermit> m_permits;
		List<Asset::MaterialUsePermit> m_materialPermits;
	};
}