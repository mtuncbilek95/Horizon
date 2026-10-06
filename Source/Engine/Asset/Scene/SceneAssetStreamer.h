#pragma once

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Asset/Scene/SceneAsset.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT SceneAssetStreamer : public AssetStreamer
	{
		HORIZON_TYPE_REFLECT(SceneAssetStreamer);
	public:
		SceneAssetStreamer() = default;
		~SceneAssetStreamer() = default;

		void OnInitialize() final;
		void OnSync(const EngineFrame& frameContext) final;
		void OnFinalize() final;

		void LoadAsync(AssetObject* pAsset) final;
		void UnloadAsync(AssetObject* pAsset) final;

		virtual Reflect::TypeHandle GetAssetType() { return Reflect::TypeOf<SceneAsset>(); }

	private:
		void RunLoadWorld(SceneAsset* pAsset);

	private:
		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_sceneLocker;
	};
}