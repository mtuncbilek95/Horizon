#pragma once

#include <Engine/Asset/AssetLoadStrategy.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Scene/SceneAsset.h>

namespace Horizon::Engine
{
	HCLASS(AssetType[Reflect::TypeOf<SceneAsset>()]);
	class H_EXPORT SceneLoadStrategy final : public AssetLoadStrategy
	{
		HORIZON_TYPE_REFLECT(SceneLoadStrategy);
	public:
		Asset* Create(List<u8>&& payload) final;
		void Destroy(Asset* pAsset) final;

		b8 Activate(Asset* pAsset) final;
		void Deactivate(Asset* pAsset) final;

		Reflect::TypeHandle GetWorkingAssetHandle() final { return Reflect::TypeOf<SceneAsset>(); }

	protected:
		void OnRegister(Engine* pEngine) final;
		void OnUnregister(Engine* pEngine) final;

	private:
		b8 Parse(SceneAsset& scene);
	};
}