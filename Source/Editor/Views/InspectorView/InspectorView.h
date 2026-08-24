#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Engine/World/World.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_EYE, "Inspector", false, true, DockZone::Right]);
	class H_EXPORT InspectorView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InspectorView);
	public:
		void OnInvoke() final;
		void OnRender() final;

		void SetInspectingEntity(Engine::EntityHandle handl);

	private:
		// TODO: This will eventually change, its only to test.
		Engine::World* m_activeWorld = nullptr;
		Engine::EntityHandle m_selectedEntity;
	};
}