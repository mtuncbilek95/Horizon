#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Query/GfxQueryHeapDesc.h>

namespace Horizon::RHI
{
	class GfxQueryHeap : public GfxObject
	{
	public:
		const GfxQueryHeapDesc& GetDesc() const { return m_desc; }
		u32 GetCount() const { return m_desc.count; }

	protected:
		GfxQueryHeapDesc m_desc{};
	};
}
