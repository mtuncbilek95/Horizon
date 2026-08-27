#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/Views/PropertyDrawer/PropertyDrawerRegistry.h>
#include <Engine/World/WorldService.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_EYE, "Inspector", false, true, DockZone::Right]);
	class H_EXPORT InspectorView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InspectorView);
	public:
		void OnInvoke() final;
		void OnRender() final;

	private:
		void DrawComponent(Engine::ComponentObject* pComponent);
		void DrawAddComponentPopup();

		b8 DrawObject(void* pInstance, Reflect::Type* pType, u32 depth);
		b8 DrawField(const PropertyContext& context);
		b8 DrawNested(const PropertyContext& context);
		b8 DrawPrimitive(const PropertyContext& context);

	private:
		Engine::World* m_currentWorld = nullptr;
		Engine::EntityHandle m_selected;

		PropertyDrawerRegistry m_drawerRegistry;
		EditState m_editState;

		Math::Vec2f m_popupPos;
		f32 m_PopupWidth = 0.0f;
		b8 m_FocusSearch = false;
		c8 m_SearchBuf[256];
	};
}