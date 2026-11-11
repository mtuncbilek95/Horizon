#include "RenderSettingsView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Systems/Rendering/RenderSubmission.h>
#include <Engine/Rendering/Forward+/ForwardPass.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void RenderSettingsView::OnInvoke()
	{
	}

	void RenderSettingsView::OnRender(const Engine::EngineFrame& context)
	{
		if (!ImGui::CollapsingHeader("Editor Debug Features", ImGuiTreeNodeFlags_DefaultOpen))
			return;

		/*ImGui::Checkbox("Show Grid", &m_showGrid);
		ImGui::Checkbox("Show Point Light Probes")*/
	}
}