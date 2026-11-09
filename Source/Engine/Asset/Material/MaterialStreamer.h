#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Material/Material.h>
#include <Engine/Asset/Material/MaterialProperties.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Asset
{
	HCLASS();
	class MaterialStreamer : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(MaterialStreamer);
	public:
		MaterialStreamer() = default;
		~MaterialStreamer() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<Material>(); }

		void LoadAsync(Material* pAsset);
		void UnloadAsync(Material* pAsset);

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
		PAL::CriticalSection m_uploadLock;
		Engine::JobSystem* m_jobSystem = nullptr;
	};
}