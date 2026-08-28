#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferDesc.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	class GfxBuffer : public GfxObject
	{
	public:
		virtual void* Map() = 0;
		virtual void Unmap() = 0;
		const GfxBufferDesc& GetDesc() const { return m_desc; }
		u64 GetDeviceAddress() const { return m_deviceAddress; }
		u32 GetShaderView() const { return m_shaderView; }
		u32 GetStorageView() const { return m_storageView; }
	protected:
		GfxBufferDesc m_desc{};
		u64 m_deviceAddress = 0;

		u32 m_shaderView = kInvalid32;
		u32 m_storageView = kInvalid32;
	};
}
