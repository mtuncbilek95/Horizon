#pragma once

#include <Editor/ToolBar/ToolBarSection.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT ToolBarItemAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ToolBarItemAttribute);
	public:
		ToolBarItemAttribute(ToolBarSection section, i32 order) : m_section(section), m_order(order)
		{
		}
		~ToolBarItemAttribute() = default;

		ToolBarSection GetSection() const { return m_section; }
		i32 GetOrder() const { return m_order; }

	private:
		ToolBarSection m_section;
		i32 m_order;
	};
}