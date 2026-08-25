#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Engine/World/WorldService.h>

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
		void DrawAddComponentPopup();

		b8 DrawVec3(const std::string& label, f32* pValues);
		b8 DrawAxisField(const std::string& axis, f32* pValue, u32 accentColor, f32 width);

	private:
		Engine::World* m_currentWorld = nullptr;
		Engine::EntityHandle m_selected;

		f32 m_PopupPosX = 0, m_PopupPosY = 0;
		f32 m_PopupWidth = 0.0f;
		b8 m_FocusSearch = false;
		c8 m_SearchBuf[256];
	};
}