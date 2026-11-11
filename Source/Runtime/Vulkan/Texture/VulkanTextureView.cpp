#include "VulkanTextureView.h"
#include <Runtime/Vulkan/Device/VulkanDevice.h>

namespace Horizon::RHI
{
	VulkanTextureView::~VulkanTextureView()
	{
		if (m_view)
			vkDestroyImageView(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), m_view, nullptr);
	}
}