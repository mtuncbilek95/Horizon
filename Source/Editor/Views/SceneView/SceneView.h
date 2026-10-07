#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Engine
{
	class RenderSystem;
	class WorldService;
	class Scene;
	class CameraComponent;
	class TransformComponent;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_MOUNTAIN_SUN, "Scene View", false, true]);
	class EDITOR_API SceneView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneView);
	public:
		SceneView() = default;
		~SceneView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		b8 IsFullBleed() const { return true; }

	private:
		Engine::CameraComponent* EnsureEditorCamera(Engine::Scene* pScene);
		void UpdateFreeRoam(const Engine::EngineFrame& context, Engine::TransformComponent& transform);
		void RenderGizmo(const Math::Vec2f& imageMin, const Math::Vec2f& imageSize, const Engine::CameraComponent& camera);

	private:
		Engine::RenderSystem* m_renderer = nullptr;
		Engine::WorldService* m_world = nullptr;
		Engine::EntityHandle m_editorCamera;

		b8 m_flying = false;
		f32 m_flySpeed = 5.0f;
		f32 m_lookSensitivity = 0.003f;

		f32 m_fpsElapsed = 0;
		f32 m_fpsFrameCount = 0;
		f32 m_fpsInterval = 0.5f;
		f32 m_fps = 0;
	};
}