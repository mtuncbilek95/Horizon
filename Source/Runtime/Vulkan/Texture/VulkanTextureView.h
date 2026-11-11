#pragma once

#include <Runtime/RHI/Texture/TextureView.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanTextureView final : public TextureView
	{
		friend class VulkanDevice;
	public:
		~VulkanTextureView() final;

		VkImageView GetHandle() const { return m_view; }
		const VkImageViewCreateInfo& GetCreateInfo() const { return m_createInfo; }
	private:
		VkImageViewCreateInfo m_createInfo {}; // for descriptor heap
		VkImageView m_view = VK_NULL_HANDLE;
	};
}