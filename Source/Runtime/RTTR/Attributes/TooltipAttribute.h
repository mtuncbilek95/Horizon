#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT TooltipAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(TooltipAttribute);
	public:
		TooltipAttribute(const std::string& tt) : m_tooltip(tt)
		{
		}
		~TooltipAttribute() = default;

		const std::string& GetTooltip() const { return m_tooltip; }

	private:
		std::string m_tooltip;
	};
}