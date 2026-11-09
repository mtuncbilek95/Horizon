#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_WRENCH, "Render Settings", DockZone::Right, EditorViewFlags::OpenOnStart]);
	class RenderSettingsView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(RenderSettingsView);
	public:
		~RenderSettingsView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
	};
}