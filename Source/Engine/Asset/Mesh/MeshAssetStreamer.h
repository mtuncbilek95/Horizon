#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/MeshAssetProperties.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Buffer/GfxBufferArena.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API MeshAssetStreamer : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(MeshAssetStreamer);
	public:
		MeshAssetStreamer() = default;
		~MeshAssetStreamer() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<MeshAsset>(); }

		void LoadAsync(MeshAsset* pAsset);
		void UnloadAsync(MeshAsset* pAsset);

		RHI::GfxBuffer* GetVertexBuffer() const { return m_vertexArena->GetBuffer(); }
		RHI::GfxBuffer* GetIndexBuffer() const { return m_indexArena->GetBuffer(); }
		u32 GetVertexBufferView() const { return m_vertexBufferView; }

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
		void RunLoadAsset(MeshAsset* pAsset);
		void ReleaseAsset(MeshAsset* pAsset);

	private:
		RHI::GfxDevice* m_device = nullptr;
		RHI::GfxDescriptorHeap* m_resourceHeap = nullptr;
		u32 m_vertexBufferView = kInvalid32;

		RHI::GfxBufferArena* m_vertexArena = nullptr;
		u8* m_vertexMap = nullptr;
		RHI::GfxBufferArena* m_indexArena = nullptr;
		u8* m_indexMap = nullptr;

		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_arenaLock;
	};
}