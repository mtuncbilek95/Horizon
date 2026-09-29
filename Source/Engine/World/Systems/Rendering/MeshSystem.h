#pragma once

#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/World/System.h>
#include <Engine/World/Components/Rendering/MeshComponent.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[9000]);
	class H_EXPORT MeshSystem : public System
	{
		HORIZON_TYPE_REFLECT(MeshSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::General; }

	private:
		void ClearResidency(MeshComponent& mesh);
		void BuildDrawRanges(MeshComponent& mesh, const MeshAsset* pAsset);

	private:
		AssetService* m_assetService = nullptr;
	};
}