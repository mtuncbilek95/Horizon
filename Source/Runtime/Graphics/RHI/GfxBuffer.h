#pragma once

#include <Runtime/Graphics/RHI/GfxObject.h>

#include <string_view>

namespace Horizon
{
	struct GfxBufferDesc
	{
		usize size = 0;
		u32 stride = 0;
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GPU;
		std::string_view debugName = "";
	};

	class GfxBuffer : public GfxObject
	{
	public:
		GfxBuffer(const GfxBufferDesc& desc, GfxDevice* pDevice);
		~GfxBuffer();

		usize GetSize() const { return m_desc.size; }
		u32 GetStride() const { return m_desc.stride; }
		GfxBufferUsage GetUsage() const { return m_desc.usage; }
		GfxMemoryType GetMemoryType() const { return m_desc.memory; }
		std::string_view GetDebugName() const { return m_desc.debugName; }
		u32 GetShaderView() const { return m_shaderView; }
		u32 GetAccessView() const { return m_accessView; }
		
		void Write(const void* pData, usize count, usize offset) const;

	private:
		GfxBufferDesc m_desc;
		GfxHandle m_mapped = nullptr;
		u64 m_gpuAddress = 0;
		u32 m_shaderView = ~0u;
		u32 m_accessView = ~0u;
	};
}