#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Engine
{
	class H_EXPORT ComponentTypeAttribute final : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ComponentTypeAttribute);
	public:
		ComponentTypeAttribute(const std::string& compName, const std::string& catName, b8 visibleOnSystem) : m_compName(compName), m_catName(catName), m_visibleOnSystem(visibleOnSystem)
		{
		}
		~ComponentTypeAttribute() = default;

		const std::string& GetComponentName() const { return m_compName; }
		const std::string& GetCategory() const { return m_catName; }
		b8 GetVisibleOnSystem() const { return m_visibleOnSystem; }

	private:
		std::string m_compName;
		std::string m_catName;
		b8 m_visibleOnSystem = false;
	};
}