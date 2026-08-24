#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Upload/GfxUploadRange.h>
#include <Runtime/RHI/Upload/GfxUploadRingDesc.h>

namespace Horizon::RHI
{
	class GfxFence;

	class GfxUploadRing : public GfxObject
	{
	public:
		virtual GfxUploadRange Allocate(usize size, usize alignment = 256) = 0;
		virtual void MarkFrame(GfxFence* pFence, u64 fenceValue) = 0;

		const GfxUploadRingDesc& GetDesc() const { return m_desc; }
		GfxBuffer* GetBuffer() const { return m_buffer; }
		usize GetCapacity() const { return m_desc.capacity; }
		usize GetUsedBytes() const { return m_usedBytes; }

	protected:
		GfxUploadRingDesc m_desc{};
		GfxBuffer* m_buffer = nullptr;
		usize m_usedBytes = 0;
	};
}