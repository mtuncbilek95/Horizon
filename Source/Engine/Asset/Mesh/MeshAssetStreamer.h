#pragma once

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/MeshProperties.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Buffer/GfxBufferArena.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT MeshAssetStreamer : public AssetStreamer
	{
		HORIZON_TYPE_REFLECT(MeshAssetStreamer);
	public:
		MeshAssetStreamer() = default;
		~MeshAssetStreamer() = default;

		void OnInitialize() final;
		void OnFinalize() final;

		void LoadAsync(AssetObject* pAsset) final;
		void UnloadAsync(AssetObject* pAsset) final;

		Reflect::TypeHandle GetAssetType() final { return Reflect::TypeOf<MeshAsset>(); }

		RHI::GfxBuffer* GetVertexBuffer() const { return m_vertexArena->GetBuffer(); }
		RHI::GfxBuffer* GetIndexBuffer() const { return m_indexArena->GetBuffer(); }

	private:
		void RunLoadAsset(MeshAsset* pAsset);
		void FailAsset(MeshAsset* pAsset, std::string_view reason);

	private:
		RHI::GfxDevice* m_device = nullptr;

		RHI::GfxBufferArena* m_vertexArena = nullptr;
		u8* m_vertexMap = nullptr;
		RHI::GfxBufferArena* m_indexArena = nullptr;
		u8* m_indexMap = nullptr;

		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_arenaLock;
	};
}