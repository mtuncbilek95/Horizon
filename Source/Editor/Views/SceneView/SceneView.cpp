#include "SceneView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/Systems/Render/RenderSystem.h>
#include <Engine/World/Systems/Location/CameraSystem.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void SceneView::OnInvoke()
	{
		auto* pWorldService = GetContext()->pEngine->RequestService<Engine::WorldService>();

		if (pWorldService == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "WorldService is unavailable, scene view stays empty");
			return;
		}

		m_renderer = pWorldService->RequestSystem<Engine::RenderSystem>();
		m_camera = pWorldService->RequestSystem<Engine::CameraSystem>();
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

		ImGui::Image(ImTextureID(handle), area);
	}
}