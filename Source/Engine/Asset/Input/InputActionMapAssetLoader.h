#pragma once

#include <Engine/Asset/Input/InputActionMapAsset.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API InputActionMapAssetLoader : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(InputActionMapAssetLoader);
	public:
		Reflect::TypeHandle GetWorkingAssetType() const { return Reflect::TypeOf<InputActionMapAsset>(); }

		void LoadSync(InputActionMapAsset* pAsset);
		void UnloadSync(InputActionMapAsset* pAsset);

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
	};
}