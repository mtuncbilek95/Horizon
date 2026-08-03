#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	class H_EXPORT RangeAttribute : public Reflect::Attribute
	{
	public:
		RangeAttribute(f32 minimum, f32 maximum) : m_min(minimum), m_max(maximum)
		{
		}
		~RangeAttribute() = default;

		f32 GetMin() const { return m_min; }
		f32 GetMax() const { return m_max; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<RangeAttribute>(); }

	private:
		f32 m_min;
		f32 m_max;
	};
}