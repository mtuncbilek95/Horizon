#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Mesh/Mesh.h>
#include <Engine/Asset/Mesh/MeshProperties.h>

#include <Runtime/Containers/Memory/RangeAllocator.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Fence/Fence.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::RHI
{
	class Device;
	class DescriptorHeap;
}

namespace Horizon::Asset
{
	HCLASS();
	class MeshStreamer : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(MeshStreamer);
	public:
		MeshStreamer() = default;
		~MeshStreamer() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<Mesh>(); }

		void LoadAsync(Mesh* pAsset);
		void UnloadAsync(Mesh* pAsset);

		RHI::Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
		RHI::Buffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
		u32 GetVertexBufferView() const { return m_vertexBufferView; }

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
		void RunLoadAsset(Mesh* pAsset);
		void ReleaseAsset(Mesh* pAsset);
		b8 UploadPayload(const u8* pVertexData, const Range& vertexRange, const u8* pIndexData, const Range& indexRange);

	private:
		RHI::Device* m_device = nullptr;
		RHI::DescriptorHeap* m_resourceHeap = nullptr;

		UniquePtr<RHI::Buffer> m_vertexBuffer;
		UniquePtr<RHI::Buffer> m_indexBuffer;
		u32 m_vertexBufferView = kInvalid32;

		RangeAllocator m_vertexRanges;
		RangeAllocator m_indexRanges;
		PAL::CriticalSection m_arenaLock;

		UniquePtr<RHI::Fence> m_uploadFence;
		PAL::CriticalSection m_uploadLock;

		Engine::JobSystem* m_jobSystem = nullptr;
	};
}