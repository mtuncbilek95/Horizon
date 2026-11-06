#pragma once

#include <Runtime/RHI/Command/CommandPoolDesc.h>
#include <Runtime/RHI/Object/Object.h>

namespace Horizon::RHI
{
	class CommandPool : public Object
	{
	public:
		virtual void Reset() = 0;

		QueueType GetType() const { return m_type; }

	protected:
		QueueType m_type = QueueType::Graphics;
	};
}
