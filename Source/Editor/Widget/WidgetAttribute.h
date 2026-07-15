#pragma once

#include <Editor/Widget/DockLayout.h>
#include <Runtime/Reflection/TypeAttribute.h>

namespace Horizon
{
	class WidgetTypeAttribute : public TypeAttribute
	{
	public:
		WidgetTypeAttribute(std::string_view name, std::string_view icon, DockLayout dock, b8 alwaysOpenFirst)
			: m_name(name), m_icon(icon), m_dock(dock), m_alwaysOpenFirst(alwaysOpenFirst)
		{
		}

		std::string_view GetDisplayName() const { return m_name; }
		std::string_view GetIcon() const { return m_icon; }
		DockLayout GetDockLayout() const { return m_dock; }
		b8 IsAlwaysOpenFirst() const { return m_alwaysOpenFirst; }

		ReflectionTypeHandle GetTypeId() const final { return TypeIdOf<WidgetTypeAttribute>(); }

	private:
		std::string_view m_name;
		std::string_view m_icon;
		DockLayout m_dock;
		b8 m_alwaysOpenFirst = false;
	};
}