#include "GfxVkDescriptorBuffer.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Util/VkBufferUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkDescriptorUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkMemoryUtils.h>

#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>

namespace Horizon
{
	GfxVkDescriptorBuffer::GfxVkDescriptorBuffer(const GfxDescriptorBufferDesc& desc, GfxDevice* pDevice)
		: GfxDescriptorBuffer(desc, pDevice), m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_deviceAddress(0)
	{
		VkDeviceSize alignment = static_cast<GfxVkDevice*>(pDevice)->DescBufferAlignment();
		VkDeviceSize finalSize = (desc.sizeInBytes + alignment - 1) & ~(alignment - 1);

		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = finalSize;
		bufferInfo.usage = VkBufferUtils::GetVkDescBufferUsage(desc.type);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VmaUtils::GetVmaUsage(desc.memUsage);
		allocInfo.flags = VmaUtils::GetVmaFlags(desc.allocFlags);

		VkResult result = vmaCreateBuffer(VmaAllocator(Root()->Allocator()), &bufferInfo, &allocInfo, &m_buffer, &m_allocation, &m_allocationInfo);
		if (result != VK_SUCCESS)
		{
			Log::Terminal(LogType::Warning, "GfxVkDescriptorBuffer::GfxVkDescriptorBuffer", "BufferSize: {}", desc.sizeInBytes);
			VDebug::VkAssert(result, "GfxVkDescriptorBuffer");
		}

		VkBufferDeviceAddressInfo addressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
		addressInfo.buffer = m_buffer;
		m_deviceAddress = vkGetBufferDeviceAddress(VkDevice(Root()->Device()), &addressInfo);
	}

	GfxVkDescriptorBuffer::~GfxVkDescriptorBuffer()
	{
		if (m_buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(VmaAllocator(Root()->Allocator()), m_buffer, m_allocation);
			m_buffer = VK_NULL_HANDLE;
			m_allocation = VK_NULL_HANDLE;
			m_allocationInfo.pMappedData = nullptr;
			m_deviceAddress = 0;
		}
	}

	void GfxVkDescriptorBuffer::RequestPayload(GfxObject* pObject, usize offsetInBytes)
	{
		auto* pDevice = static_cast<GfxVkDevice*>(Root());
		VkDeviceSize payloadSize = pDevice->DescTypeSize(UseType());

		VkDescriptorGetInfoEXT getInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
		getInfo.type = VkDescriptorUtils::GetVkDescType(UseType());

		VkDescriptorAddressInfoEXT addrInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };

		if (UseType() == DescriptorType::Storage || UseType() == DescriptorType::DynamicStorage)
		{
			auto* buffer = static_cast<GfxBuffer*>(pObject);
			addrInfo.address = buffer->DeviceAddress();
			addrInfo.range = buffer->SizeInBytes();
			addrInfo.format = VK_FORMAT_UNDEFINED;

			getInfo.data.pStorageBuffer = &addrInfo; 
		}
		else if (UseType() == DescriptorType::Uniform || UseType() == DescriptorType::DynamicUniform)
		{
			auto* buffer = static_cast<GfxBuffer*>(pObject);
			addrInfo.address = buffer->DeviceAddress();
			addrInfo.range = buffer->SizeInBytes();
			addrInfo.format = VK_FORMAT_UNDEFINED;

			getInfo.data.pUniformBuffer = &addrInfo;
		}

		uint8_t* pTarget = static_cast<uint8_t*>(MappedData()) + offsetInBytes;
		vkGetDescriptorEXT(VkDevice(Root()->Device()), &getInfo, payloadSize, pTarget);
	}

	void GfxVkDescriptorBuffer::WriteDescriptor(usize offsetInBytes, const void* pDescriptorData, usize descriptorSize)
	{
		if (m_allocationInfo.pMappedData)
		{
			u8* dst = static_cast<u8*>(m_allocationInfo.pMappedData) + offsetInBytes;
			std::memcpy(dst, pDescriptorData, descriptorSize);
			return;
		}

		void* mapped = nullptr;
		VDebug::VkAssert(vmaMapMemory(VmaAllocator(Root()->Allocator()), m_allocation, &mapped), "GfxVkDescriptorBuffer::WriteDescriptor");

		u8* dst = static_cast<u8*>(mapped) + offsetInBytes;
		std::memcpy(dst, pDescriptorData, descriptorSize);

		vmaUnmapMemory(VmaAllocator(Root()->Allocator()), m_allocation);
	}

	u64 GfxVkDescriptorBuffer::DeviceAddress() const
	{
		return m_deviceAddress;
	}

	void* GfxVkDescriptorBuffer::MappedData() const
	{
		return m_allocationInfo.pMappedData;
	}
}