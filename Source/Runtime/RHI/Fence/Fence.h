#pragma once

#include <Runtime/RHI/Fence/FenceDesc.h>
#include <Runtime/RHI/Object/Object.h>

namespace Horizon::RHI
{
	class Fence : public Object
	{
	public:
		virtual u64 GetCompletedValue() = 0;
		virtual void WaitCPU(u64 value) = 0;

		b8 IsComplete(u64 value) { return GetCompletedValue() >= value; }
		u64 GetSignaledValue() const { return m_value; }

	protected:
		u64 m_value = 0;
	};
}
