#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Scene/SceneAsset.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API SceneAssetStreamer : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(SceneAssetStreamer);
	public:
		SceneAssetStreamer() = default;
		~SceneAssetStreamer() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<SceneAsset>(); }

		void LoadAsync(SceneAsset* pAsset);
		void UnloadAsync(SceneAsset* pAsset);

		void LoadSync(SceneAsset* pAsset);
		void UnloadSync(SceneAsset* pAsset);

	protected:
		void OnInitialize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
		void RunLoadWorld(SceneAsset* pAsset);

	private:
		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_sceneLocker;
	};
}