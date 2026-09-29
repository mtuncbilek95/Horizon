#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT RangeAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(RangeAttribute);
	public:
		RangeAttribute(f32 minimum, f32 maximum) : m_min(minimum), m_max(maximum)
		{
		}
		~RangeAttribute() = default;

		f32 GetMin() const { return m_min; }
		f32 GetMax() const { return m_max; }

	private:
		f32 m_min;
		f32 m_max;
	};
}