#pragma once

#include <Editor/Views/DockZone.h>
#include <Editor/Views/EditorViewFlags.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class EDITOR_API EditorViewAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(EditorViewAttribute);
	public:
		EditorViewAttribute(const std::string& iconName, const std::string& displayName, DockZone dock = DockZone::Center, 
			EditorViewFlags flags = EditorViewFlags::OpenOnStart) : m_dock(dock), m_flags(flags)
		{
			m_displayName = iconName + " " + displayName;
		}

		~EditorViewAttribute() = default;

		const std::string& GetDisplayName() const { return m_displayName; }
		EditorViewFlags GetFlags() const { return m_flags; }
		DockZone GetDock() const { return m_dock; }

	private:
		std::string m_displayName;
		EditorViewFlags m_flags;
		DockZone m_dock;
	};
}