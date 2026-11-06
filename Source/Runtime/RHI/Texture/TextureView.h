#pragma once

#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Texture/TextureViewDesc.h>

namespace Horizon::RHI
{
	class TextureView : public Object
	{
	public:
		Texture* GetTexture() const { return m_texture; }

		TextureViewType GetType() const { return m_type; }
		TextureFormat GetFormat() const { return m_format; }
		TextureAspect GetAspect() const { return m_aspect; }

		u32 GetBaseMip() const { return m_baseMip; }
		u32 GetMipCount() const { return m_mipCount; }
		u32 GetBaseLayer() const { return m_baseLayer; }
		u32 GetLayerCount() const { return m_layerCount; }

	protected:
		Texture* m_texture = nullptr;

		TextureViewType m_type = TextureViewType::Texture2D;
		TextureFormat m_format = TextureFormat::Undefined;
		TextureAspect m_aspect = TextureAspect::All;

		u32 m_baseMip = 0;
		u32 m_mipCount = 1;
		u32 m_baseLayer = 0;
		u32 m_layerCount = 1;
	};
}