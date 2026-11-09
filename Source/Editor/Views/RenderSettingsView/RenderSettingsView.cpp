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
		auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();

		World::System::RenderSubmission* pSys = (World::System::RenderSubmission*)pWorld->FindSystem(Reflect::TypeOf<World::System::RenderSubmission>());

		static b8 fPassClicked = true;
		ImGui::Checkbox("ForwardPass", &fPassClicked);

		pSys->GetGraph()->SetEnablePass(Reflect::TypeOf<Rendering::Pass::ForwardPass>(), fPassClicked);
	}
}