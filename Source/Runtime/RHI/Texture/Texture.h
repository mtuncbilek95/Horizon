#pragma once

#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Texture/TextureDesc.h>

namespace Horizon::RHI
{
	class Texture : public Object
	{
	public:
		TextureType GetType() const { return m_type; }
		TextureFormat GetFormat() const { return m_format; }
		TextureUsage GetUsage() const { return m_usage; }

		u32 GetWidth() const { return m_width; }
		u32 GetHeight() const { return m_height; }
		u32 GetDepth() const { return m_depth; }
		u32 GetArrayLayers() const { return m_arrayLayers; }
		u32 GetMipLevels() const { return m_mipLevels; }

		SampleCount GetSampleCount() const { return m_sampleCount; }

	protected:
		TextureType m_type = TextureType::Texture2D;
		TextureFormat m_format = TextureFormat::Undefined;
		TextureUsage m_usage = TextureUsage::None;

		u32 m_width = 1;
		u32 m_height = 1;
		u32 m_depth = 1;
		u32 m_arrayLayers = 1;
		u32 m_mipLevels = 1;

		SampleCount m_sampleCount = SampleCount::Count1;
	};
}