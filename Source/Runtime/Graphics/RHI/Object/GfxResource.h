#pragma once

#include <atomic>

namespace Horizon
{
	class GfxResource
	{
		friend class GfxDevice;
	public:
		void IncrementPointer() { m_refCount.fetch_add(1, std::memory_order_relaxed); }
		void Release();

	protected:
		virtual ~GfxResource() = default;

	protected:
		std::atomic<u32> m_refCount = 1;
		GfxDevice* m_ownerDevice = nullptr;
	};
}