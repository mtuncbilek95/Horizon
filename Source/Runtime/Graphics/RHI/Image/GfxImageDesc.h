#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/MemoryFormat.h>

namespace Horizon
{
	struct GfxImageDesc
	{
		Math::Vec3u imgSize = { 0, 0, 1 };
		u32 mipLevels = 1;
		u32 arrayLayers = 1;
		ImageType type = ImageType::Image2D;
		ImageFormat format = ImageFormat::R8G8B8A8_UNorm;
		ImageUsage usage = ImageUsage::ColorAttachment;

		MemoryUsage memUsage = MemoryUsage::AutoPreferDevice;
		MemoryAllocation memFlags = MemoryAllocation::DedicatedMemory;

		GfxImageDesc& setSize(const Math::Vec3u& size) { imgSize = size; return *this; }
		GfxImageDesc& setMipLevels(u32 mips) { mipLevels = mips; return *this; }
		GfxImageDesc& setArrayLayers(u32 layers) { arrayLayers = layers; return *this; }
		GfxImageDesc& setType(ImageType t) { type = t; return *this; }
		GfxImageDesc& setFormat(ImageFormat fmt) { format = fmt; return *this; }
		GfxImageDesc& setUsage(ImageUsage u) { usage = u; return *this; }
		GfxImageDesc& setMemoryUsage(MemoryUsage mu) { memUsage = mu; return *this; }
		GfxImageDesc& setMemoryFlags(MemoryAllocation mf) { memFlags = mf; return *this; }
	};

	struct GfxViewDesc
	{
		ImageAspect aspect = ImageAspect::Color;
		u32 baseMip = 0;
		u32 baseArray = 0;

		GfxViewDesc& setAspect(ImageAspect asp) { aspect = asp; return *this; }
		GfxViewDesc& setBaseMip(u32 mip) { baseMip = mip; return *this; }
		GfxViewDesc& setBaseArray(u32 array) { baseArray = array; return *this; }
	};
}
