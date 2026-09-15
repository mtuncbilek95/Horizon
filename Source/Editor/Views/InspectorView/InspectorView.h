#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/Components/ComponentDrawer.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class WorldService;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_EYE, "Inspector", false, true, DockZone::Right]);
	class H_EXPORT InspectorView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InspectorView);
	public:
		~InspectorView();

		void OnInvoke() final;
		void OnRender() final;

	private:
		Engine::ReflectionSystem* m_reflSys;
		Engine::WorldService* m_worldService;

		List<ComponentDrawer*> m_drawerList;
		std::unordered_map<Engine::ComponentTypeId, usize> m_drawerLookups;
	};
}