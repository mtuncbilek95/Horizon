#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Texture/TextureAsset.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API TextureAssetStreamer : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(TextureAssetStreamer);
	public:
		static constexpr u32 ShaderViewCapacity = 4096;

	public:
		TextureAssetStreamer() = default;
		~TextureAssetStreamer() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<TextureAsset>(); }

		void LoadAsync(TextureAsset* pAsset);
		void UnloadAsync(TextureAsset* pAsset);

		void LoadSync(TextureAsset* pAsset);
		void UnloadSync(TextureAsset* pAsset);

	protected:
		void OnInitialize() final;
		void OnFinalize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
		void RunLoadTexture(TextureAsset* pAsset);
		void ReleaseAsset(TextureAsset* pAsset);

	private:
		RHI::GfxDevice* m_device = nullptr;
		RHI::GfxQueue* m_queue = nullptr;
		RHI::GfxFence* m_fence = nullptr;
		RHI::GfxCommandList* m_commandList = nullptr;
		RHI::GfxDescriptorHeap* m_shaderHeap = nullptr;

		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_textureLocker;
	};
}