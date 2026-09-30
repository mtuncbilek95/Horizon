#include "PlayControlsItem.h"

#include <Editor/Font/IconsFontAwesome6.h>
#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>

#include <imgui.h>

namespace Horizon::Editor
{
	namespace
	{
		static const Math::Color4f sPlayColor = Math::Color4f(0.20f, 0.65f, 0.30f, 1.0f);
		static const Math::Color4f sPauseColor = Math::Color4f(0.85f, 0.65f, 0.15f, 1.0f);
		static const Math::Color4f sStopColor = Math::Color4f(0.80f, 0.25f, 0.25f, 1.0f);

		ImVec4 ToImVec4(const Math::Color4f& color, f32 scale)
		{
			return ImVec4(color.R() * scale, color.G() * scale, color.B() * scale, color.A());
		}
	}

	void PlayControlsItem::OnRender()
	{
		if (StateButton(ICON_FA_PLAY, m_state == State::Play, sPlayColor) && m_state != State::Play)
		{
			auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();
			pWorld->SetRunningSystems(Engine::SceneGroups::Play);
			m_state = State::Play;
		}

		ImGui::SameLine();

		if (StateButton(ICON_FA_PAUSE, m_state == State::Pause, sPauseColor) && m_state == State::Play)
		{
			auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();
			pWorld->SetRunningSystems(Engine::SceneGroups::Pause);
			m_state = State::Pause;
		}

		ImGui::SameLine();

		if (StateButton(ICON_FA_STOP, m_state == State::Stop, sStopColor) && m_state != State::Stop)
		{
			auto* pWorld = GetContext()->pEngine->RequestService<Engine::WorldService>();
			pWorld->SetRunningSystems(Engine::SceneGroups::Edit);
			m_state = State::Stop;
		}
	}

	b8 PlayControlsItem::StateButton(const c8* pIcon, b8 active, const Math::Color4f& activeCol)
	{
		if (active)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(activeCol, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(activeCol, 1.15f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(activeCol, 0.85f));
		}

		const b8 pressed = ImGui::Button(pIcon);

		if (active)
			ImGui::PopStyleColor(3);

		return pressed;
	}

}