#include "PlayControlsItem.h"

#include <Editor/Font/IconsFontAwesome6.h>
#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void PlayControlsItem::OnRender()
	{
		if (ImGui::Button(ICON_FA_PLAY) && m_state != State::Play)
		{
			auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();
			pWorld->SetRunningSystems(Engine::SceneGroups::Play);
			m_state = State::Play;
		}

		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_PAUSE) && m_state == State::Play)
		{
			auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();
			pWorld->SetRunningSystems(Engine::SceneGroups::Pause);
			m_state = State::Pause;
		}

		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_STOP) && m_state != State::Stop)
		{
			auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();
			pWorld->SetRunningSystems(Engine::SceneGroups::Edit);
			m_state = State::Stop;
		}
	}
}