#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	struct GfxTextureDesc
	{
		u32 width = 1, height = 1;
		u32 depth = 1, mipLevels = 1;
		u32 arraySize = 1;

		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		GfxTextureUsage usage = GfxTextureUsage::Sampled;
		GfxTextureType type = GfxTextureType::Tex2D;
		GfxTextureTypeFlags typeFlags = GfxTextureTypeFlags::None;
		GfxSampleCount sampleCount = GfxSampleCount::Count1;

		GfxColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};

	class GfxTexture : public GfxObject
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