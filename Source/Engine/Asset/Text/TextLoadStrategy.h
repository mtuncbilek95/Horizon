#pragma once

#include <Engine/Asset/AssetLoadStrategy.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Text/TextAsset.h>

namespace Horizon::Engine
{
	HCLASS(AssetType[Reflect::TypeOf<TextAsset>()]);
	class H_EXPORT TextLoadStrategy final : public AssetLoadStrategy
	{
		HORIZON_TYPE_REFLECT(TextLoadStrategy);
	public:
		Asset* Create(const List<u8>& payload) final;
		void Destroy(Asset* pAsset) final;

		b8 Activate(Asset* pAsset) final;
		void Deactivate(Asset* pAsset) final;

		Reflect::TypeHandle GetWorkingAssetHandle() final { return Reflect::TypeOf<TextAsset>(); }

	protected:
		void OnRegister(Engine* pEngine) final;
		void OnUnregister(Engine* pEngine) final;

	private:
	};
}