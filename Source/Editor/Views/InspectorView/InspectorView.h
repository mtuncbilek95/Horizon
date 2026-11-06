#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_EYE, "Inspector", DockZone::Right, EditorViewFlags::OpenOnStart]);
	class InspectorView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InspectorView);
	public:
		~InspectorView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
	};
}