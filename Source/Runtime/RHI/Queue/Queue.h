#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Queue/QueueDesc.h>
#include <Runtime/RHI/Object/Object.h>

namespace Horizon::RHI
{
	class CommandList;
	class Fence;

	class Queue : public Object
	{
	public:
		virtual void Submit(List<CommandList*> cmdlists) = 0;
		virtual u64 Signal(Fence* pFence) = 0;
		virtual void Wait(Fence* pFence, u64 value) = 0;

		virtual u64 GetTimestampFrequency() = 0;

		QueueType GetType() const { return m_type; }

	protected:
		QueueType m_type = QueueType::Graphics;
	};
}