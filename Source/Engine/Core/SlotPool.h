#pragma once

#include <vector>
#include <functional>

namespace Horizon
{
	template<typename TRecord, typename THandle>
	class SlotPool
	{
	public:
		THandle Create(const TRecord& record)
		{
			u32 index;
			if (!m_freeList.empty())
			{
				index = m_freeList.back();
				m_freeList.pop_back();
			}
			else
			{
				index = u32(m_slots.size());
				m_slots.push_back({});
			}

			m_slots[index].record = record;
			m_slots[index].alive = true;
			return THandle::Generate(index, m_slots[index].generation);
		}

		TRecord* Resolve(THandle handle)
		{
			if (!handle.IsValid() || handle.Index() >= m_slots.size())
				return nullptr;

			Slot& slot = m_slots[handle.Index()];
			if (!slot.alive || slot.generation != handle.Generation())
				return nullptr;

			return &slot.record;
		}

		void Destroy(THandle handle)
		{
			TRecord* pRecord = Resolve(handle);
			if (pRecord == nullptr)
				return;

			Slot& slot = m_slots[handle.Index()];
			slot.alive = false;
			slot.generation++;
			slot.record = {};
			m_freeList.push_back(handle.Index());
		}

		void DestroyWith(THandle handle, std::function<void(TRecord*)>&& func)
		{
			TRecord* pRecord = Resolve(handle);

			if (pRecord == nullptr)
				return;

			func(pRecord);
			Destroy(handle);
		}

	private:
		struct Slot
		{
			TRecord record;
			u32 generation = 0;
			b8 alive = false;
		};

		std::vector<Slot> m_slots;
		std::vector<u32> m_freeList;
	};
}