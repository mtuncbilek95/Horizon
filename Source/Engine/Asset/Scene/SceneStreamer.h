#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Scene/Scene.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Asset
{
	HCLASS();
	class SceneStreamer : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(SceneStreamer);
	public:
		SceneStreamer() = default;
		~SceneStreamer() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<Scene>(); }

		void LoadAsync(Scene* pAsset);
		void UnloadAsync(Scene* pAsset);

		void LoadSync(Scene* pAsset);
		void UnloadSync(Scene* pAsset);

	protected:
		void OnInitialize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
		void RunLoadWorld(Scene* pAsset);

	private:
		Engine::JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_sceneLocker;
	};
}