#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

namespace Horizon
{
	struct GfxTextureDesc
	{
		u32 width = 1, height = 1;
		u32 depth = 1, mipLevels = 1;

		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		GfxTextureUsage usage = GfxTextureUsage::Sampled;
		GfxTextureType type = GfxTextureType::Tex2D;
	};

	class GfxTexture : public GfxResource
	{
	public:
		const GfxTextureDesc& GetDesc() const { return m_desc; }

		u32 GetShaderView() const { return m_shaderView; }
		u32 GetAccessView() const { return m_accessView; }

		GfxResourceState GetState() const { return m_state; }
		void SetState(GfxResourceState state) { m_state = state; }

	protected:
		GfxTextureDesc m_desc{};
		GfxResourceState m_state = GfxResourceState::Common;

		u32 m_shaderView = kInvalid32;
		u32 m_accessView = kInvalid32;
	};
}