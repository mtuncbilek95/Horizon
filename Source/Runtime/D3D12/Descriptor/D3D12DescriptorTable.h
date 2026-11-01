#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Descriptor/DescriptorTable.h>
#include <Runtime/RHI/Descriptor/DescriptorType.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12DescriptorTable final : public DescriptorTable
	{
		friend class D3D12Device;

		struct Slot
		{
			u32 binding = 0;
			DescriptorType type = DescriptorType::Texture;
			u32 offset = 0;
			u32 count = 0;
		};

	public:
		~D3D12DescriptorTable() final;

		void Write(u32 binding, u32 arrayIndex, TextureView* pView) final;
		void Write(u32 binding, u32 arrayIndex, Buffer* pBuffer, u64 offset, u64 size, u32 stride = 0) final;
		void Write(u32 binding, u32 arrayIndex, Sampler* pSampler) final;

		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle() const;

	private:
		const Slot* FindSlot(u32 binding, u32 arrayIndex) const;

	private:
		List<Slot> m_slots;
		u32 m_offset = 0;
		u32 m_count = 0;
	};
}