#include "FrameGraphCache.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>

namespace Horizon
{
	FrameGraphCache::FrameGraphCache(GfxDevice* pDevice) : m_device(pDevice)
	{}

	FrameGraphCache::~FrameGraphCache()
	{
		m_caches.clear();

		if (m_device)
			m_device = nullptr;
	}

	RenderTargetEntry& FrameGraphCache::GetOrCreate(const RenderTargetDesc& desc)
	{
		auto [begin, end] = m_caches.equal_range(desc);

		for (auto it = begin; it != end; ++it)
		{
			if (!it->second.isInUse)
			{
				it->second.isInUse = true;
				return it->second;
			}
		}

		auto entry = m_caches.emplace(desc, CreateRenderTarget(desc));
		entry->second.isInUse = true;
		return entry->second;
	}

	RenderTargetEntry FrameGraphCache::CreateRenderTarget(const RenderTargetDesc& desc)
	{
		RenderTargetEntry entry = {};

		entry.img = m_device->CreateImage(GfxImageDesc()
			.setSize({ desc.size.x, desc.size.y, 1 })
			.setMipLevels(1)
			.setArrayLayers(1)
			.setType(ImageType::Image2D)
			.setFormat(desc.format)
			.setUsage(desc.usage | ImageUsage::TransferSrc)
			.setMemoryUsage(MemoryUsage::AutoPreferDevice)
			.setMemoryFlags(MemoryAllocation::DedicatedMemory));

		entry.view = m_device->CreateView(GfxImageViewDesc()
			.setImage(entry.img.get())
			.setViewType(ImageViewType::View2D)
			.setAspect(HasFlag(desc.usage, ImageUsage::DepthStencil) ? ImageAspect::Depth : ImageAspect::Color)
			.setBaseMip(0)
			.setBaseArray(0));

		return entry;
	}
}