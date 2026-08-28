#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferDesc.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	/**
	 * @brief Graphics buffer to represent all types
	 * of data such as Vertex, Index, Constant/Uniform,
	 * Storage etc.
	 * 
	 * @code
	 *   RHI::GfxBufferDesc bufDesc = {};
	 *   RHI::GfxBuffer* buf = nullptr;
	 *   buf = myRhiDevice->CreateBuffer(bufDesc);
	 * @endcode
	 */
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
