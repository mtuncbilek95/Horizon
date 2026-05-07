#pragma once

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <array>

namespace Horizon
{
	class RenderBatch
	{
	public:
		void Submit(std::shared_ptr<GfxCommandBuffer> cb, u32 layer = 0)
		{
			m_entries.push_back({ std::move(cb), layer });
		}

		std::vector<GfxCommandBuffer*> Flush(u32 frameIndex)
		{
			m_inFlight[frameIndex].clear();

			std::sort(m_entries.begin(), m_entries.end(),
				[](const Entry& a, const Entry& b) { return a.layer < b.layer; });

			std::vector<GfxCommandBuffer*> result;
			result.reserve(m_entries.size());
			for (auto& e : m_entries)
				result.push_back(e.cb.get());

			m_inFlight[frameIndex] = std::move(m_entries);
			m_entries.clear();

			return result;
		}

	private:
		struct Entry
		{
			std::shared_ptr<GfxCommandBuffer> cb;
			u32 layer;
		};

		std::vector<Entry> m_entries;
		std::array<std::vector<Entry>, 2> m_inFlight;
	};
}