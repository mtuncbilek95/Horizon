#pragma once

#include <Engine/Asset/Input/InputActionMap.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Asset
{
	HCLASS();
	class InputActionMapLoader : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(InputActionMapLoader);
	public:
		InputActionMapLoader() = default;
		~InputActionMapLoader() = default;

		InputActionMapLoader(const InputActionMapLoader&) = delete;
		InputActionMapLoader& operator=(const InputActionMapLoader&) = delete;

		Reflect::TypeHandle GetWorkingAssetType() const { return Reflect::TypeOf<InputActionMap>(); }

		void LoadSync(InputActionMap* pAsset);
		void UnloadSync(InputActionMap* pAsset);

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
	};
}