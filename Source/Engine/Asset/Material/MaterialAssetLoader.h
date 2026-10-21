#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Material/MaterialAsset.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	class AssetService;

	HCLASS();
	class ENGINE_API MaterialAssetLoader : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(MaterialAssetLoader);
	public:
		MaterialAssetLoader() = default;
		~MaterialAssetLoader() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<MaterialAsset>(); }

		void LoadAsync(MaterialAsset* pAsset);
		void UnloadAsync(MaterialAsset* pAsset);

		void LoadSync(MaterialAsset* pAsset);
		void UnloadSync(MaterialAsset* pAsset);

	protected:
		void OnInitialize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
		void ReleaseAsset(MaterialAsset* pAsset);

	private:
		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_materialLocker;
	};
}