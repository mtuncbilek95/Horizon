#pragma once

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Runtime/Containers/List.h>
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
		void OnFinalize() final;

		Reflect::TypeHandle GetAssetType() final { return Reflect::TypeOf<MeshAsset>(); }

		AssetObject* Load(const AssetEntry& entry) final;
		void Unload(AssetObject* pObject) final;

	private:
		b8 ReadCooked(const AssetEntry& entry, MeshProperties& outProperties, List<u8>& outPayload);
		RHI::GfxBuffer* CreateFilledBuffer(RHI::GfxBufferUsage usage, const u8* pData, usize size, u32 stride);

	private:
		RHI::GfxDevice* m_device = nullptr;
		MeshResourceCache* m_pool = nullptr;
	};
}