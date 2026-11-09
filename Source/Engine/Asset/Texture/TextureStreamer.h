#pragma once

#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Asset
{
	HCLASS();
	class TextureStreamer : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(TextureStreamer);
	public:
		Reflect::TypeHandle GetWorkingAssetType() const { return Reflect::TypeOf<Texture>(); }

		void LoadAsync(Texture* pAsset);
		void UnloadAsync(Texture* pAsset);

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
	};
}