#include "MainScreen.h"

#include <imgui.h>

namespace Horizon
{
	void MainScreen::Draw(u64 sceneTextureId)
	{
		DrawDockSpace();

		DrawViewport(sceneTextureId);

		ImGui::Begin("Hierarchy");
		ImGui::End();

		ImGui::Begin("Inspector");
		ImGui::End();

		ImGui::Begin("Console");
		ImGui::End();
	}

	void MainScreen::DrawDockSpace()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		const ImGuiWindowFlags hostFlags =
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Horizon##MainScreen", nullptr, hostFlags);
		ImGui::PopStyleVar(3);

		const ImGuiID dockspaceId = ImGui::GetID("HorizonDockSpace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		DrawMenuBar();

		ImGui::End();
	}

	void MainScreen::DrawMenuBar()
	{
		if (!ImGui::BeginMenuBar())
			return;

		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Exit");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	void MainScreen::DrawViewport(u64 sceneTextureId)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		const ImVec2 size = ImGui::GetContentRegionAvail();
		if (sceneTextureId != 0 && size.x > 0.0f && size.y > 0.0f)
			ImGui::Image((ImTextureID)sceneTextureId, size);

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
