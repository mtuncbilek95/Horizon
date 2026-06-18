#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandList.h>

#include <vector>
#include <memory>
#include <mutex>

namespace Horizon
{
	class GfxDevice;

	class CommandListPool
	{
	public:
		void Init(GfxDevice* pDevice, GfxQueueType type);

		GfxCommandList* Acquire();
		void Reset();

	private:
		GfxDevice* m_device = nullptr;
		GfxQueueType m_type = GfxQueueType::Graphics;

		std::vector<std::unique_ptr<GfxCommandList>> m_lists;
		usize m_next = 0;
		std::mutex m_mutex;
	};
}