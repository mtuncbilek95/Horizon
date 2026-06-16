#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

namespace Horizon
{
	struct GfxBufferDesc
	{
		usize size = 0;
		u32 stride = 0;
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GPU;
	};

	class GfxBuffer : public GfxResource
	{
	public:
		const GfxBufferDesc& GetDesc() const { return m_desc; }

		u32 GetShaderView() const { return m_shaderView; }
		u32 GetAccessView() const { return m_accessView; }
		u64 GetGpuAddress() const { return m_gpuAddress; }

		void Write(const void* pData, usize size, usize offset);

	protected:
		GfxBufferDesc m_desc;
		void* m_mapped = nullptr;
		u64 m_gpuAddress = 0;
		u32 m_shaderView = kInvalid32;
		u32 m_accessView = kInvalid32;
	};
}