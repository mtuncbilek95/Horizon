#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Descriptor/DescriptorTable.h>
#include <Runtime/RHI/Descriptor/DescriptorType.h>

namespace Horizon::RHI
{
	class VulkanDescriptorTable final : public DescriptorTable
	{
		friend class VulkanDevice;

		struct Slot
		{
			DescriptorType type = DescriptorType::Texture;
			u32 offset = 0;
			u32 count = 0;
		};

	public:
		~VulkanDescriptorTable() final;

		void Write(u32 binding, u32 arrayIndex, TextureView* pView) final;
		void Write(u32 binding, u32 arrayIndex, Buffer* pBuffer, u64 offset, u64 size, u32 stride = 0) final;
		void Write(u32 binding, u32 arrayIndex, Sampler* pSampler) final;

		u64 GpuHandle() const;
		u64 GetOffset() const { return m_offset; }

	private:
		const Slot* FindSlot(u32 binding, u32 arrayIndex) const;

	private:
		List<Slot> m_slots;
		u32 m_offset = 0;
		u32 m_count = 0;
	};
}