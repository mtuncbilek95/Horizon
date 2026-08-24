#pragma once

#include <Engine/Asset/AssetLoadStrategy.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Mesh/StaticMeshAsset.h>

namespace Horizon::Engine
{
	HCLASS(AssetType[Reflect::TypeOf<StaticMeshAsset>()]);
	class H_EXPORT StaticMeshLoadStrategy final : public AssetLoadStrategy
	{
		HORIZON_TYPE_REFLECT(StaticMeshLoadStrategy);
	public:
		Asset* Create(const List<u8>& payload) final;
		void Destroy(Asset* pAsset) final;

		b8 Activate(Asset* pAsset) final;
		void Deactivate(Asset* pAsset) final;

		Reflect::TypeHandle GetWorkingAssetHandle() final { return Reflect::TypeOf<StaticMeshAsset>(); }

	protected:
		void OnRegister(Engine* pEngine) final;
		void OnUnregister(Engine* pEngine) final;

	private:
	};
}