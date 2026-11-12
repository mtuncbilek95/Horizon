#pragma once

#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Asset/Texture/TextureContext.h>
#include <Engine/Asset/Texture/TextureProperties.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Fence/Fence.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	class JobSystem;
}

namespace Horizon::Asset
{
	HCLASS();
	class TextureStreamer : public Engine::AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(TextureStreamer);

		struct RetiredContext
		{
			UniquePtr<TextureContext> context;
			u64 retiredTick = 0;
		};

		static constexpr u64 kRetireTicks = 3;

	public:
		TextureStreamer() = default;
		~TextureStreamer() = default;

		TextureStreamer(const TextureStreamer&) = delete;
		TextureStreamer& operator=(const TextureStreamer&) = delete;

		Reflect::TypeHandle GetWorkingAssetType() const { return Reflect::TypeOf<Texture>(); }

		void LoadAsync(Texture* pAsset);
		void UnloadAsync(Texture* pAsset);

	protected:
		void OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& frame) final;
		void OnFinalize() final;

		void OnRegisteredAsset(const Engine::AssetEntry& entry) final;
		void OnUnregisteredAsset(const Engine::AssetEntry& entry) final;
		void OnRefreshAsset(const Engine::AssetEntry& entry) final;

	private:
		void RunLoadAsset(Texture* pAsset);
		void ReleaseAsset(Texture* pAsset);
		b8 UploadPayload(const u8* pPayload, usize payloadSize, const List<TextureMip>& mips, RHI::Texture* pTexture);
	private:
		RHI::Device* m_device = nullptr;
		Engine::JobSystem* m_jobSystem = nullptr;

		UniquePtr<RHI::Fence> m_uploadFence = nullptr;
		PAL::CriticalSection m_lock;
		List<UniquePtr<TextureContext>> m_availableContexts;
		List<RetiredContext> m_retiredContexts;
		u64 m_tick = 0;

		RHI::DescriptorHeap* m_resourceHeap = nullptr;
	};
}