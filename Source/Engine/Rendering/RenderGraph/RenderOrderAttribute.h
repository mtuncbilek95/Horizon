#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class RenderOrderAttribute final : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(RenderOrderAttribute);
	public:
		RenderOrderAttribute(i32 order) : m_order(order)
		{
		}
		~RenderOrderAttribute() = default;

		i32 GetOrder() const { return m_order; }

	private:
		i32 m_order;
	};
}