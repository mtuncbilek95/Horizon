#pragma once

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class MeshResourceCache;

	HCLASS();
	class H_EXPORT MeshAssetStreamer : public AssetStreamer
	{
		HORIZON_TYPE_REFLECT(MeshAssetStreamer);
	public:
		MeshAssetStreamer() = default;
		~MeshAssetStreamer() = default;

		void OnInitialize() final;
		void OnPublish() final;
		void OnFinalize() final;

		Reflect::TypeHandle GetAssetType() final { return Reflect::TypeOf<MeshAsset>(); }

	private:
		MeshResourceCache* m_pool = nullptr;
	};
}