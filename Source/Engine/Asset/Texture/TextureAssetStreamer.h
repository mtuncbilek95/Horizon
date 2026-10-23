#pragma once

#include <Engine/Asset/Texture/TextureAsset.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API TextureAssetStreamer : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(TextureAssetStreamer);
	public:
		Reflect::TypeHandle GetWorkingAssetType() const { return Reflect::TypeOf<TextureAsset>(); }

		void LoadAsync(TextureAsset* pAsset);
		void UnloadAsync(TextureAsset* pAsset);

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
	};
}