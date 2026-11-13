#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Material/Material.h>
#include <Engine/Asset/Material/MaterialData.h>
#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Asset/Texture/TextureUsePermit.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/CriticalSection.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Sampler/Sampler.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::RHI
{
	class Device;
	class DescriptorHeap;
}

namespace Horizon::Engine
{
	class AssetService;
	class JobSystem;
}

namespace Horizon::Asset
{
	HCLASS();
	class MaterialStreamer : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(MaterialStreamer);

		static constexpr u32 kMaxMaterialCount = 4096;
		static constexpr u32 kTextureSlotCount = 6;

		struct SlotPermits
		{
			TextureUsePermit permits[kTextureSlotCount];
		};

	public:
		MaterialStreamer() = default;
		~MaterialStreamer() = default;

		MaterialStreamer(const MaterialStreamer&) = delete;
		MaterialStreamer& operator=(const MaterialStreamer&) = delete;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<Material>(); }

		void LoadAsync(Material* pAsset);
		void UnloadAsync(Material* pAsset);

		void AcquireTextures(Material* pAsset);
		void ReleaseTextures(Material* pAsset);

		RHI::Buffer* GetMaterialBuffer() const { return m_materialBuffer.Get(); }
		u32 GetMaterialBufferView() const { return m_materialBufferView; }
		u32 GetDefaultSamplerView() const { return m_defaultSamplerView; }

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
		void RunLoadAsset(Material* pAsset);
		void ReleaseAsset(Material* pAsset);

		MaterialData BuildData(const MaterialParameters& parameters) const;
		void WriteSlot(u32 slotIndex, const MaterialData& data);
		u32 ResolveTexture(Engine::AssetService* pAssets, Engine::AssetHandle<Texture>& handle, TextureUsePermit& permit);

	private:
		RHI::Device* m_device = nullptr;
		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		Engine::JobSystem* m_jobSystem = nullptr;

		UniquePtr<RHI::Buffer> m_materialBuffer;
		u32 m_materialBufferView = kInvalid32;

		RHI::DescriptorHeap* m_samplerHeap = nullptr;
		UniquePtr<RHI::Sampler> m_defaultSampler;
		u32 m_defaultSamplerView = kInvalid32;

		PAL::CriticalSection m_lock;
		List<u32> m_freeSlots;
		List<SlotPermits> m_slotPermits;
		u32 m_slotCount = 0;
	};
}