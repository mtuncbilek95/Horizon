#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT TooltipAttribute : public Reflect::Attribute
	{
	public:
		TooltipAttribute(const std::string& tt) : m_tooltip(tt)
		{
		}
		~TooltipAttribute() = default;

		const std::string& GetTooltip() const { return m_tooltip; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<TooltipAttribute>(); }

	private:
		std::string m_tooltip;
	};
}