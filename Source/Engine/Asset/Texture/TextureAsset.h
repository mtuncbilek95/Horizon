#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/Texture/TextureAssetProperties.h>
#include <Engine/Job/JobTicket.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RHI/Texture/GfxTexture.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetKind::Streamable]);
	class ENGINE_API TextureAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(TextureAsset);
		friend class TextureAssetStreamer;
	public:
		TextureAsset() = default;
		~TextureAsset() = default;

		void LoadSync();
		void UnloadSync();

		const Guid& GetId() const { return m_id; }
		AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

		const TextureAssetProperties& GetProperties() const { return m_properties; }
		RHI::GfxTexture* GetTexture() const { return m_texture; }
		u32 GetShaderIndex() const { return m_shaderIndex; }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;

		TextureAssetProperties m_properties;
		RHI::GfxTexture* m_texture = nullptr;
		u32 m_shaderIndex = kInvalid32;

		SubmitTicket m_ticket;
	};
}