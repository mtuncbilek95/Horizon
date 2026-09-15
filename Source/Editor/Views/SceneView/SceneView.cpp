#include "SceneView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>
#include <Editor/Models/SelectionModel.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/Components/CameraComponent.h>
#include <Engine/World/Components/LocalToWorldComponent.h>
#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/Systems/Render/RenderSystem.h>
#include <Engine/World/Systems/Location/CameraSystem.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include <cmath>

namespace Horizon::Editor
{
	namespace
	{
		ImGuizmo::OPERATION GuizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE GuizmoMode = ImGuizmo::WORLD;
	}

	void SceneView::OnInvoke()
	{
		m_world = GetContext()->pEngine->RequestService<Engine::WorldService>();

		if (m_world == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "WorldService is unavailable, scene view stays empty");
			return;
		}

		m_renderer = m_world->RequestSystem<Engine::RenderSystem>();
		m_camera = m_world->RequestSystem<Engine::CameraSystem>();

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

	void SceneView::OnRender()
	{
		if (m_renderer == nullptr)
		{
			ImGui::TextDisabled("No render system");
			return;
		}

		const ImVec2 area = ImGui::GetContentRegionAvail();

		if (area.x < 1.0f || area.y < 1.0f)
			return;

		const Math::Vec2u requested(u32(area.x), u32(area.y));

		if (m_renderer->GetImageSize() != requested)
		{
			m_renderer->ResizeImage(requested);
			m_camera->ResizeViewport(requested);
		}

		const u64 handle = m_renderer->GetSceneView();

		if (handle == kInvalid64)
			return;

		const ImVec2 imageMin = ImGui::GetCursorScreenPos();
		ImGui::Image(ImTextureID(handle), area);

		RenderGizmo(imageMin, area);
	}

	void SceneView::RenderGizmo(const ImVec2& imageMin, const ImVec2& imageSize)
	{
		auto* pSelection = GetContext()->pSelection;

		if (!pSelection->Is<Engine::EntityTag>())
			return;

		Engine::Scene* pScene = m_world->GetCurrentWorld();
		const Engine::EntityHandle entity = pSelection->Get<Engine::EntityTag>();

		auto* pTransform = pScene->FindComponent<Engine::TransformComponent>(entity);
		auto* pLocalToWorld = pScene->FindComponent<Engine::LocalToWorldComponent>(entity);

		if (pTransform == nullptr || pLocalToWorld == nullptr)
			return;

		const Engine::CameraComponent* pCamera = nullptr;
		const Engine::LocalToWorldComponent* pCamTransform = nullptr;

		pScene->ForEach<Engine::CameraComponent, Engine::LocalToWorldComponent>([&](Engine::EntityHandle, Engine::CameraComponent& cameraMatrix, Engine::LocalToWorldComponent& transform)
			{
				if (pCamera == nullptr)
					pCamera = &cameraMatrix;

				if (pCamTransform == nullptr)
					pCamTransform = &transform;
			});

		if (pCamera == nullptr)
			return;

		if (pCamTransform == nullptr)
			return;

		if (ImGui::IsWindowHovered() && !ImGuizmo::IsUsing())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_W))
				GuizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_E))
				GuizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_R))
				GuizmoOperation = ImGuizmo::SCALE;
		}

		ImGuizmo::SetRect(imageMin.x, imageMin.y, imageSize.x, imageSize.y);
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetGizmoSizeClipSpace(0.15f);
		ImGuizmo::SetAxisLimit(0.0025f);
		ImGuizmo::SetPlaneLimit(0.02f);

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(imageMin.x, imageMin.y, imageSize.x, imageSize.y);

		Math::Mat4f entityWorld = pLocalToWorld->m_worldMatrix;

		Math::Mat4f viewMat = pCamTransform->m_worldMatrix.Inverse();
		Math::Mat4f projMat = Math::Mat4f::Perspective(Math::DegToRad(pCamera->m_fov), imageSize.x / imageSize.y, pCamera->m_nearPlane, pCamera->m_farPlane);

		const b8 changed = ImGuizmo::Manipulate(viewMat.Data(), projMat.Data(), GuizmoOperation, GuizmoMode, entityWorld.Data());

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