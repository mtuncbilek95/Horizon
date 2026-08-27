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

		b8 IsFullBleed() const final { return true; }

	private:
		void RenderToolbar(const ImVec2& imageMin, const ImVec2& region);
		void RenderDebugPopup();

	private:
		Engine::WorldRenderSystem* m_renderSystem = nullptr;

		b8 m_wireframe = false;
		b8 m_bounds = false;
		b8 m_grid = false;
		b8 m_colliders = false;
	};
}