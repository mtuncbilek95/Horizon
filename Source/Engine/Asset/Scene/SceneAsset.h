#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetKind::Streamable]);
	class ENGINE_API SceneAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(SceneAsset);
		friend class SceneAssetStreamer;
	public:
		SceneAsset();
		~SceneAsset();

		void LoadAsync();
		void UnloadAsync();

		void LoadSync();
		void UnloadSync();

		const Guid& GetId() const { return m_id; }
		AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }
		Scene* GetScene() const { return m_scene; }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;
		SubmitTicket m_ticket = InvalidSubmitTicket;
		Scene* m_scene = nullptr;
	};
}