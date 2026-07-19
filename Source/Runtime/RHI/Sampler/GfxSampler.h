#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	class GfxSampler : public GfxObject
	{
	public:
		const GfxSamplerDesc& GetDesc() const { return m_desc; }

		u32 GetHeapIndex() const { return m_heapIndex; }

	protected:
		GfxSamplerDesc m_desc{};
		u32 m_heapIndex = kInvalid32;
	};
}