#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class ENGINE_API SystemOrderAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(SystemOrderAttribute);
	public:
		SystemOrderAttribute(u32 order) : m_order(order) {}
		~SystemOrderAttribute() = default;

		u32 GetOrderNumber() const { return m_order; }

	private:
		u32 m_order = 0;
	};
}