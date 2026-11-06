#pragma once

#include <Runtime/RHI/Buffer/BufferDesc.h>
#include <Runtime/RHI/Object/Object.h>

namespace Horizon::RHI
{
	class Buffer : public Object
	{
	public:
		u64 GetSize() const { return m_size; }
		
		BufferUsage GetUsage() const { return m_usage; }
		MemoryType GetMemoryType() const { return m_memory; }
		
		u64 GetGpuAddress() const { return m_gpuAddress; }
		u8* GetMappedData() const { return m_mappedData; }

		b8 IsMapped() const { return m_mappedData != nullptr; }

	protected:
		u64 m_size = 0;

		BufferUsage m_usage = BufferUsage::None;
		MemoryType m_memory = MemoryType::GpuOnly;

		u64 m_gpuAddress = 0;
		u8* m_mappedData = nullptr;
	};
}