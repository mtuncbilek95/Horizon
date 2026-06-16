#include "GfxBuffer.h"

#include <cassert>

namespace Horizon
{
	void GfxBuffer::Write(const void* pData, usize size, usize offset)
	{
		assert(m_mapped && "Buffer is not CPU-visible");
		std::memcpy((u8*)m_mapped + offset, pData, size);
	}
}