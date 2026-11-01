#pragma once

#include <Runtime/RHI/Descriptor/DescriptorHeapDesc.h>
#include <Runtime/RHI/Descriptor/DescriptorType.h>
#include <Runtime/RHI/Object/Object.h>

namespace Horizon::RHI
{
	class TextureView;
	class Buffer;
	class Sampler;

	class RUNTIME_API DescriptorHeap : public Object
	{
	public:
		virtual u32 Allocate() = 0;
		virtual void Free(u32 index) = 0;

		virtual void Write(u32 index, DescriptorType type, TextureView* pView) = 0;
		virtual void Write(u32 index, DescriptorType type, Buffer* pBuffer, u64 offset, u64 size, u32 stride = 0) = 0;
		virtual void Write(u32 index, Sampler* pSampler) = 0;

		virtual u64 GetGpuHandle(u32 index) const = 0;

		DescriptorHeapType GetType() const { return m_type; }
		u32 GetCapacity() const { return m_capacity; }

	protected:
		DescriptorHeapType m_type = DescriptorHeapType::Resource;
		u32 m_capacity = 0;
	};
}