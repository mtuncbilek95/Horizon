#include "SceneView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>
#include <Editor/Models/SelectionModel.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/Components/Rendering/CameraComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>
#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/Systems/Rendering/RenderSystem.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/SystemGroup.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec3f.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include <cmath>

namespace Horizon::Editor
{
	namespace
	{
		ImGuizmo::OPERATION GuizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE GuizmoMode = ImGuizmo::WORLD;

		constexpr f32 PitchLimit = Math::HalfPi - 0.01f;
		constexpr f32 BoostMultiplier = 3.0f;
		constexpr f32 MinFlySpeed = 0.1f;
		constexpr f32 MaxFlySpeed = 500.0f;
	}

	void SceneView::OnInvoke()
	{
		m_world = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_world->SetRunningSystems(Engine::SceneGroups::Edit);

		if (m_world == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "WorldService is unavailable, scene view stays empty");
			return;
		}

		m_renderer = m_world->RequestSystem<Engine::RenderSystem>();

		ImGuizmo::Style& style = ImGuizmo::GetStyle();
		style.TranslationLineThickness = 3.0f;
		style.TranslationLineArrowSize = 6.0f;
		style.RotationLineThickness = 4.0f;
		style.RotationOuterLineThickness = 5.0f;
		style.ScaleLineThickness = 3.0f;
		style.ScaleLineCircleSize = 6.0f;
		style.CenterCircleSize = 6.0f;

		style.Colors[ImGuizmo::PLANE_X] = ImVec4(0.9f, 0.2f, 0.2f, 1.f);
		style.Colors[ImGuizmo::PLANE_Y] = ImVec4(0.2f, 0.9f, 0.2f, 1.f);
		style.Colors[ImGuizmo::PLANE_Z] = ImVec4(0.2f, 0.2f, 0.9f, 1.f);
	}

	void SceneView::OnRender(const Engine::EngineFrame& context)
	{
		if (m_renderer == nullptr)
		{
			ImGui::TextDisabled("No render system");
			return;
		}

		Engine::Scene* pScene = m_world->GetCurrentWorld();
		if (pScene == nullptr)
		{
			ImGui::TextDisabled("No active world");
			return;
		}

		const ImVec2 area = ImGui::GetContentRegionAvail();
		if (area.x < 1.0f || area.y < 1.0f)
			return;

		Engine::CameraComponent* pCamera = EnsureEditorCamera(pScene);
		if (pCamera == nullptr)
			return;

		auto* pCamTransform = pScene->FindComponent<Engine::TransformComponent>(m_editorCamera);
		if (pCamTransform == nullptr)
			return;

		const Math::Vec2f requested = { area.x, area.y };

		if (pCamera->m_targetScreen != requested)
			pCamera->m_targetScreen = requested;

		const u64 handle = m_renderer->GetSceneView();

		if (handle == kInvalid64)
			return;

		const ImVec2 imageMin = ImGui::GetCursorScreenPos();
		const ImVec2 imageMax = ImVec2(imageMin.x + area.x, imageMin.y + area.y);

		const b8 hasSelection = GetContext()->pSelection->Is<Engine::EntityTag>();
		const b8 gizmoWantsMouse = hasSelection && (ImGuizmo::IsOver() || ImGuizmo::IsUsing());

		if (m_flying || !gizmoWantsMouse)
		{
			ImGui::InvisibleButton("##scene_input", area, ImGuiButtonFlags_MouseButtonRight);
			m_flying = ImGui::IsItemActive();
		}
		else
		{
			ImGui::Dummy(area);
			m_flying = false;
		}

		ImGui::GetWindowDrawList()->AddImage(ImTextureID(handle), imageMin, imageMax);

		if (m_flying)
			UpdateFreeRoam(context, *pCamTransform);

		ImGui::SetCursorScreenPos(ImVec2(imageMin.x + 24, imageMin.y + 24));

		m_fpsElapsed += context.DeltaTime();
		m_fpsFrameCount++;

		if (m_fpsElapsed >= m_fpsInterval)
		{
			m_fps = static_cast<f32>(m_fpsFrameCount) / m_fpsElapsed;
			m_fpsElapsed = 0.0f;
			m_fpsFrameCount = 0;
		}

		std::string fpsCounter = std::format("FPS: {:.2f}", m_fps);
		ImGui::TextColored(ImVec4(0.1, 0.8, 0.2, 1), fpsCounter.c_str());

		if(m_world->GetRunningSystems() == Engine::SceneGroups::Edit)
			RenderGizmo({ imageMin.x, imageMin.y }, { area.x, area.y }, *pCamera);
	}

	Engine::CameraComponent* SceneView::EnsureEditorCamera(Engine::Scene* pScene)
	{
		if (m_editorCamera.IsValid() && pScene->IsAlive(m_editorCamera))
			return pScene->FindComponent<Engine::CameraComponent>(m_editorCamera);

		m_editorCamera = pScene->AddEntity();

		if (!m_editorCamera.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "Editor camera entity could not be created");
			return nullptr;
		}

		pScene->AddComponent(m_editorCamera, Engine::EditorOnlyComponent());

		Engine::TransformComponent transform = {};
		transform.m_position = Math::Vec3f(0.f, 2.f, 6.f);
		transform.m_rotation = Math::Vec3f(Math::DegToRad(-15.f), 0.f, 0.f);
		pScene->AddComponent(m_editorCamera, std::move(transform));

		return pScene->AddComponent(m_editorCamera, Engine::CameraComponent());
	}

	void SceneView::UpdateFreeRoam(const Engine::EngineFrame& context, Engine::TransformComponent& transform)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);

		const ImGuiIO& io = ImGui::GetIO();

		f32 pitch = transform.m_rotation.X() - io.MouseDelta.y * m_lookSensitivity;
		f32 yaw = transform.m_rotation.Y() - io.MouseDelta.x * m_lookSensitivity;
		pitch = Math::Clamp(pitch, -PitchLimit, PitchLimit);

		transform.m_rotation = Math::Vec3f(pitch, yaw, 0.f);

		if (io.MouseWheel != 0.f)
			m_flySpeed = Math::Clamp(m_flySpeed * (1.f + io.MouseWheel * 0.1f), MinFlySpeed, MaxFlySpeed);

		const Math::Quat orientation = Math::Quat::FromEuler(transform.m_rotation);
		const Math::Vec3f forward = orientation.RotateVector(Math::Vec3f(0.f, 0.f, -1.f));
		const Math::Vec3f right = orientation.RotateVector(Math::Vec3f(1.f, 0.f, 0.f));
		const Math::Vec3f up = Math::Vec3f(0.f, 1.f, 0.f);

		Math::Vec3f move = Math::Vec3f::Zero();

		if (ImGui::IsKeyDown(ImGuiKey_W))
			move += forward;
		if (ImGui::IsKeyDown(ImGuiKey_S))
			move -= forward;
		if (ImGui::IsKeyDown(ImGuiKey_D))
			move += right;
		if (ImGui::IsKeyDown(ImGuiKey_A))
			move -= right;
		if (ImGui::IsKeyDown(ImGuiKey_E))
			move += up;
		if (ImGui::IsKeyDown(ImGuiKey_Q))
			move -= up;

		const f32 length = std::sqrt(move | move);

		if (length < Math::Quat::KindaSmallNumber)
			return;

		const f32 speed = m_flySpeed * (io.KeyShift ? BoostMultiplier : 1.f);
		transform.m_position += (move / length) * (speed * context.DeltaTime());
	}

	void SceneView::RenderGizmo(const Math::Vec2f& imageMin, const Math::Vec2f& imageSize, const Engine::CameraComponent& camera)
	{
		if (!GetContext()->pSelection->Is<Engine::EntityTag>())
			return;

		Engine::Scene* pScene = m_world->GetCurrentWorld();
		const Engine::EntityHandle entity = GetContext()->pSelection->Get<Engine::EntityTag>();
		
		auto* pTransform = pScene->FindComponent<Engine::TransformComponent>(entity);

		if (pTransform == nullptr)
			return;

		if (ImGui::IsWindowHovered() && !ImGuizmo::IsUsing() && !m_flying)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_W))
				GuizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_E))
				GuizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_R))
				GuizmoOperation = ImGuizmo::SCALE;
		}

		ImGuizmo::SetRect(imageMin.X(), imageMin.Y(), imageSize.X(), imageSize.Y());
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetGizmoSizeClipSpace(0.15f);
		ImGuizmo::SetAxisLimit(0.0025f);
		ImGuizmo::SetPlaneLimit(0.02f);

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(imageMin.X(), imageMin.Y(), imageSize.X(), imageSize.Y());

		Math::Mat4f entityWorld = pTransform->m_worldMatrix;
		Math::Mat4f viewMat = camera.m_view;
		Math::Mat4f projMat = camera.m_projection;

		b8 changed = ImGuizmo::Manipulate(viewMat.Data(), projMat.Data(), GuizmoOperation, GuizmoMode, entityWorld.Data());

		if (!changed)
			return;

		Math::Vec3f position;
		Math::Vec3f eulerRadians;
		Math::Vec3f scale;

		entityWorld.DecomposeWorldMatrix(position, eulerRadians, scale);

		pTransform->m_position = position;
		pTransform->m_rotation = eulerRadians;
		pTransform->m_scale = scale;
	}
}