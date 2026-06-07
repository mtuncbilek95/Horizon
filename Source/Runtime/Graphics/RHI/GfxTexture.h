#pragma once

#include <Runtime/Graphics/RHI/GfxObject.h>

#include <glm/glm.hpp>

#include <string_view>

namespace Horizon
{
	struct GfxTextureDesc
	{
		u32 width = 1, height = 1;
		u32 depth = 1;
		u32 mipLevels = 1;

		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		GfxTextureUsage usage = GfxTextureUsage::Sampled;
		GfxTextureType type = GfxTextureType::Tex2D;

		glm::vec4 clearColor = { 0.f, 0.f, 0.f, 1.f };
		f32 clearDepth = 0.0f;
		u8 clearStencil = 0;

		std::string_view debugName = "";
	};

	class GfxTexture : public GfxObject
	{
	public:
		GfxTexture(const GfxTextureDesc& desc, GfxDevice* pDevice);
		GfxTexture(GfxDevice* pDevice, GfxHandle backbufferResource, const GfxTextureDesc& desc);
		~GfxTexture();

		u32 GetWidth() const { return m_desc.width; }
		u32 GetHeight() const { return m_desc.height; }
		u32 GetDepth() const { return m_desc.depth; }
		u32 GetMipLevels() const { return m_desc.mipLevels; }
		GfxTextureFormat GetFormat() const { return m_desc.format; }
		GfxTextureUsage GetUsage() const { return m_desc.usage; }
		GfxTextureType GetType() const { return m_desc.type; }
		std::string_view GetDebugName() const { return m_desc.debugName; }

		u32 GetShaderView() const { return m_shaderView; }
		u32 GetAccessView() const { return m_accessView; }
		u32 GetTargetView() const { return m_targetView; }
		u32 GetDepthView() const { return m_depthView; }

	private:
		GfxTextureDesc m_desc;
		u32 m_shaderView = ~0u;
		u32 m_accessView = ~0u;
		u32 m_targetView = ~0u;
		u32 m_depthView = ~0u;
	};
}
