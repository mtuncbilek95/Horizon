#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Horizon::Engine
{
	class RenderSystem;
	class CameraSystem;
	class WorldService;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_MOUNTAIN_SUN, "Scene View", false, true]);
	class H_EXPORT SceneView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneView);
	public:
		SceneView() = default;
		~SceneView() = default;

		void OnInvoke() final;
		void OnRender() final;

		b8 IsFullBleed() const { return true; }

	private:
		void RenderGizmo(const ImVec2& imageMin, const ImVec2& imageSize);

	private:
		Engine::RenderSystem* m_renderer = nullptr;
		Engine::CameraSystem* m_camera = nullptr;
		Engine::WorldService* m_world = nullptr;
	};
}