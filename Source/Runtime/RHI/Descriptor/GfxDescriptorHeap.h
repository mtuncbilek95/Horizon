#pragma once

#include <Runtime/RHI/Descriptor/GfxDescriptorHeapDesc.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	class GfxTexture;
	class GfxBuffer;

	/*
	* GfxDescriptorHeap owns a contiguous block of descriptors and hands out
	* indices into it. Descriptor heaps are not owned by the device; the
	* GraphicsContext creates them and everything that needs a descriptor
	* requests it from there.
	*
	* Allocate and Free deal in bare slots, while the CreateXView calls both
	* reserve a slot and write the view into it, stamping the resulting index
	* onto the resource so shaders can reach it through the bindless heap.
	*/
	class GfxDescriptorHeap : public GfxObject
	{
	public:
		virtual u32 Allocate() = 0;
		virtual void Free(u32 index) = 0;

		virtual u32 CreateShaderView(GfxTexture* pTexture) = 0;
		virtual u32 CreateStorageView(GfxTexture* pTexture) = 0;
		virtual u32 CreateRenderTargetView(GfxTexture* pTexture) = 0;
		virtual u32 CreateDepthStencilView(GfxTexture* pTexture) = 0;

		virtual u32 CreateShaderView(GfxBuffer* pBuffer) = 0;
		virtual u32 CreateStorageView(GfxBuffer* pBuffer) = 0;

		const GfxDescriptorHeapDesc& GetDesc() const { return m_desc; }
		GfxDescriptorHeapType GetType() const { return m_desc.type; }
		u32 GetCapacity() const { return m_desc.capacity; }
		u32 GetAllocatedCount() const { return m_allocatedCount; }

	protected:
		GfxDescriptorHeapDesc m_desc{};
		u32 m_allocatedCount = 0;
	};
}
