#pragma once

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Material/MaterialBlendMode.h>
#include <Engine/Rendering/MeshDrawItem.h>
#include <Engine/World/System.h>
#include <Engine/World/Components/Rendering/MeshComponent.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
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

		const List<MeshDrawItem>& GetOpaqueItems() const { return m_opaqueItems; }
		const List<MeshDrawItem>& GetMaskedItems() const { return m_maskedItems; }
		const List<MeshDrawItem>& GetTranslucentItems() const { return m_translucentItems; }
		u64 GetDrawListFrame() const { return m_drawListFrame; }

	private:
		MaterialBlendMode ResolveMaterial(const MeshComponent& mesh, u32 slot, MeshDrawItem& outItem);
		void UnloadSceneAssets();

	private:
		AssetService* m_assetService = nullptr;
		RHI::GfxDevice* m_device = nullptr;

		Scene* m_lastScene = nullptr;

		std::unordered_set<Guid> m_loadedMeshes;
		std::unordered_set<Guid> m_loadedMaterials;
		std::unordered_set<Guid> m_loadedShaders;
		std::unordered_set<Guid> m_loadedTextures;
		std::unordered_set<Guid> m_reportedAssets;

		List<MeshDrawItem> m_opaqueItems;
		List<MeshDrawItem> m_maskedItems;
		List<MeshDrawItem> m_translucentItems;
		u64 m_drawListFrame = u64_max;
	};
}