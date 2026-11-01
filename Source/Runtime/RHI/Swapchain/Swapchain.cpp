#include "Swapchain.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	Texture* Swapchain::GetImage(u32 index) const
	{
		if (index >= m_imageCount)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Image index {} is out of range, count: {}", index, m_imageCount);
			return nullptr;
		}

		return m_images[index].Get();
	}

	TextureView* Swapchain::GetImageView(u32 index) const
	{
		if (index >= m_imageCount)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Image view index {} is out of range, count: {}", index, m_imageCount);
			return nullptr;
		}

		return m_views[index].Get();
	}
}
