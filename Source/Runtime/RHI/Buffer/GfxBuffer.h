#pragma once

#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

#include <cstring>

namespace Horizon
{
	struct GfxBufferDesc
	{
		usize size = 0;
		u32 stride = 0;
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GpuOnly;
	};

	class GfxBuffer : public GfxObject
	{
	public:
		const GfxBufferDesc& GetDesc() const { return m_desc; }

		u32 GetShaderView() const { return m_shaderView; }
		u32 GetAccessView() const { return m_accessView; }
		u64 GetGpuAddress() const { return m_gpuAddress; }

		void Write(const void* pData, usize size, usize offset = 0)
		{
			if (!m_mapped)
			{
				Terminal::Error("GfxBuffer", "Write called on a non-mapped buffer, memory type must be Upload");
				return;
			}

			std::memcpy(static_cast<u8*>(m_mapped) + offset, pData, size);
		}

	protected:
		GfxBufferDesc m_desc{};
		void* m_mapped = nullptr;
		u64 m_gpuAddress = 0;
		u32 m_shaderView = kInvalid32;
		u32 m_accessView = kInvalid32;
	};
}