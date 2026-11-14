#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Reflect
{
	class UnitAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(UnitAttribute);
	public:
		UnitAttribute(const std::string& unit) : m_unit(unit)
		{
		}
		~UnitAttribute() = default;

		const std::string& GetUnit() const { return m_unit; }

	private:
		std::string m_unit;
	};
}