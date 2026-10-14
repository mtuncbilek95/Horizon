#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	class EDITOR_API ProjectSectionAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ProjectSectionAttribute);
	public:
		ProjectSectionAttribute(const std::string& displayName) : m_displayName(displayName)
		{
		}
		~ProjectSectionAttribute() = default;

		const std::string& GetDisplayName() const { return m_displayName; }

	private:
		std::string m_displayName;
	};
}