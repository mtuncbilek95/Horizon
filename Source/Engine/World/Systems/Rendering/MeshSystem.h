#pragma once

#include <Engine/Asset/AssetService.h>
#include <Engine/World/System.h>
#include <Engine/World/Components/Rendering/MeshComponent.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_set>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[SystemPhase::Mesh]);
	class ENGINE_API MeshSystem : public System
	{
		HORIZON_TYPE_REFLECT(MeshSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::General; }

	private:
		void UnloadSceneMeshes();

	private:
		AssetService* m_assetService = nullptr;
		RHI::GfxDevice* m_device = nullptr;

		Scene* m_lastScene = nullptr;
		std::unordered_set<Guid> m_loadedMeshes;
	};
}