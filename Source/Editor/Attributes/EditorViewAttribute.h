#pragma once

#include <Editor/Views/DockZone.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon
{
	class H_EXPORT EditorViewAttribute : public Reflect::Attribute
	{
	public:
		EditorViewAttribute(const std::string& iconName, const std::string& displayName, b8 multiInstance = false, b8 openOnStart = false, DockZone dock = DockZone::Center) : m_multiInstance(multiInstance), 
			m_openOnStart(openOnStart), m_dock(dock)
		{
			m_displayName = iconName + " " + displayName;
		}

		~EditorViewAttribute() = default;

		const std::string& GetDisplayName() const { return m_displayName; }
		b8 GetMultiInstance() const { return m_multiInstance; }
		b8 GetOpenOnStart() const { return m_openOnStart; }
		DockZone GetDock() const { return m_dock; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<EditorViewAttribute>(); }

	private:
		std::string m_displayName;
		b8 m_multiInstance;
		b8 m_openOnStart;
		DockZone m_dock;
	};
}