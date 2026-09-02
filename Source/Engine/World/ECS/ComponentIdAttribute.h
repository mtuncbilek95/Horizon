#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Engine
{
	class H_EXPORT ComponentIdAttribute final : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ComponentIdAttribute);
	public:
		ComponentIdAttribute(const std::string& dispName, const std::string& dispCat) : m_displayName(dispName),
			m_displayCategory(dispCat)
		{
		}
		~ComponentIdAttribute() = default;

		const std::string& GetDisplayName() const { return m_displayName; }
		const std::string& GetDisplayCategory() const { return m_displayCategory; }

	private:
		std::string m_displayName;
		std::string m_displayCategory;
	};
}