#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <imgui.h>

namespace Horizon::Engine
{
	class WorldRenderSystem;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_MOUNTAIN_SUN, "Scene View", false, true]);
	class H_EXPORT SceneView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneView);
	public:
		void OnInvoke() final;
		void OnRender() final;

	private:
	};
}