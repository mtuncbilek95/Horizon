#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Reflect
{
	class H_EXPORT AliasAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AliasAttribute);
	public:
		AliasAttribute(const std::string& formerName) : m_formerName(formerName)
		{
		}
		~AliasAttribute() = default;

		const std::string& GetFormerName() const { return m_formerName; }

	private:
		std::string m_formerName;
	};
}