#include "SceneView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Systems/WorldRenderSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr f32 kToolbarMargin = 8.0f;
	}

	void SceneView::OnInvoke()
	{
		auto* pWorldService = GetContext()->pEngine->RequestService<Engine::WorldService>();

		if (pWorldService == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "WorldService is unavailable, scene view stays empty");
			return;
		}

		m_renderSystem = pWorldService->FindSystem<Engine::WorldRenderSystem>();
	}

	void SceneView::OnRender()
	{
		if (m_renderSystem == nullptr)
		{
			ImGui::TextDisabled("No render system");
			return;
		}

		const ImVec2 region = ImGui::GetContentRegionAvail();

		if (region.x < 1.0f || region.y < 1.0f)
			return;

		m_renderSystem->RequestSize(u32(region.x), u32(region.y));

		const u64 handle = m_renderSystem->GetColorTargetHandle();

		if (handle == 0)
			return;

		const ImVec2 imageMin = ImGui::GetCursorScreenPos();

		ImGui::Image(ImTextureID(handle), region);

		RenderToolbar(imageMin, region);
	}

	void SceneView::RenderToolbar(const ImVec2& imageMin, const ImVec2& region)
	{
		ImGui::SetCursorScreenPos({ imageMin.x + kToolbarMargin, imageMin.y + kToolbarMargin });

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGuiUtils::Hex("#161616D9"));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));

		const f32 barWidth = region.x - kToolbarMargin * 2.0f;
		const f32 barHeight = ImGui::GetFrameHeight() + 8.0f;

		if (barWidth <= 1.0f)
		{
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();
			return;
		}

		const ImGuiChildFlags childFlags = ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_Borders;

		if (ImGui::BeginChild("SceneToolbar", { barWidth, barHeight }, childFlags, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (ImGui::Button(ICON_FA_PLAY))
			{
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_FA_PAUSE))
			{
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_FA_STOP))
			{
			}

			ImGui::SameLine();

			const char* pDebugLabel = ICON_FA_BUG " Debug";
			const f32 debugWidth = ImGui::CalcTextSize(pDebugLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;

			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			const f32 rightEdge = cursor.x + ImGui::GetContentRegionAvail().x;

			if (rightEdge - debugWidth > cursor.x)
				ImGui::SetCursorScreenPos({ rightEdge - debugWidth, cursor.y });

			if (ImGui::Button(pDebugLabel))
				ImGui::OpenPopup("SceneDebugPopup");


			RenderDebugPopup();
		}

		ImGui::EndChild();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	void SceneView::RenderDebugPopup()
	{
		const ImVec2 buttonMin = ImGui::GetItemRectMin();
		const ImVec2 buttonMax = ImGui::GetItemRectMax();

		ImGui::SetNextWindowPos({ buttonMin.x, buttonMax.y + 4.0f });

		if (!ImGui::BeginPopup("SceneDebugPopup"))
			return;

		ImGui::Checkbox("Wireframe", &m_wireframe);
		ImGui::Checkbox("Bounds", &m_bounds);
		ImGui::Checkbox("Grid", &m_grid);
		ImGui::Checkbox("Colliders", &m_colliders);

		ImGui::EndPopup();
	}
}