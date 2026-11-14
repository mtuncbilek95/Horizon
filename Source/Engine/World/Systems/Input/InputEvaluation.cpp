#include "InputEvaluation.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Input/InputActionMap.h>
#include <Engine/Input/InputService.h>
#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Input/InputReceiver.h>
#include <Engine/World/Components/Input/InputState.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Input/DeviceType.h>
#include <Runtime/Input/Gamepad.h>
#include <Runtime/Input/Keyboard.h>
#include <Runtime/Input/Mouse.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::World::System
{
	b8 InputEvaluation::OnInitialize()
	{
		m_assets = GetEngine()->RequestService<Engine::AssetService>();
		if (!m_assets)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "AssetService is missing, scene has no action map to evaluate");
			return false;
		}

		m_input = GetEngine()->RequestService<Engine::InputService>();
		if (!m_input)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "InputService is missing, scene has no device to evaluate");
			return false;
		}

		return true;
	}

	void InputEvaluation::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		Input::Instance* pDevices = m_input->GetDevices();
		if (!pDevices)
			return;

		Input::Keyboard* pKeyboard = nullptr;
		const usize keyboardCount = pDevices->GetObjectCount(Input::DeviceType::Keyboard);

		for (usize i = 0; i < keyboardCount; ++i)
		{
			Input::Keyboard* pCandidate = pDevices->GetKeyboard(i);

			if (pKeyboard == nullptr || pCandidate->IsAnyKeyDown())
				pKeyboard = pCandidate;
		}

		Input::Mouse* pMouse = nullptr;
		if (pDevices->GetObjectCount(Input::DeviceType::Mouse) > 0)
			pMouse = pDevices->GetMouse(0);

		const usize gamepadCount = pDevices->GetObjectCount(Input::DeviceType::Gamepad);

		currentScene.ForEach<Component::InputReceiver, Component::InputState>(
			[&](Engine::EntityHandle, Component::InputReceiver& receiver, Component::InputState& state)
			{
				Asset::InputActionMap* pMap = m_assets->Resolve(receiver.m_mapHandle);
				if (!pMap)
					return;

				if (pMap->GetResidency() == Engine::AssetResidency::Unloaded)
					pMap->LoadSync();

				if (pMap->GetResidency() != Engine::AssetResidency::Resident)
					return;

				Input::Gamepad* pGamepad = nullptr;
				if (receiver.m_playerIndex < gamepadCount)
					pGamepad = pDevices->GetGamepad(receiver.m_playerIndex);

				const List<Asset::InputAction>& actions = pMap->GetActions();
				if (state.m_actions.GetCount() != actions.GetCount())
					state.m_actions.Resize(actions.GetCount());

				for (usize i = 0; i < actions.GetCount(); ++i)
				{
					Math::Vec2f value = Math::Vec2f(0.0f, 0.0f);

					for (const Asset::InputBinding& binding : actions[i].bindings)
						value += ReadBinding(binding, pKeyboard, pMouse, pGamepad);

					Component::InputActionState& action = state.m_actions[i];

					const b8 wasDown = action.down;
					const b8 isDown = value.X() != 0.0f || value.Y() != 0.0f;

					action.value = value;
					action.down = isDown;
					action.pressed = isDown && !wasDown;
					action.released = !isDown && wasDown;
				}
			});
	}

	void InputEvaluation::OnFinalize()
	{
	}

	void InputEvaluation::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Writes<Component::InputReceiver>();
		graph.Writes<Component::InputState>();
	}

	Math::Vec2f InputEvaluation::ReadBinding(const Asset::InputBinding& binding, Input::Keyboard* pKeyboard, Input::Mouse* pMouse, Input::Gamepad* pGamepad) const
	{
		switch (binding.source)
		{
		case Asset::InputBindingSource::Key:
		{
			if (pKeyboard && pKeyboard->IsKeyDown(static_cast<PAL::KeyCode>(binding.code)))
				return binding.scale;

			break;
		}
		case Asset::InputBindingSource::MouseButton:
		{
			if (pMouse && pMouse->IsButtonDown(static_cast<PAL::MouseButton>(binding.code)))
				return binding.scale;

			break;
		}
		case Asset::InputBindingSource::MouseDelta:
		{
			if (pMouse)
				return pMouse->GetDelta() * binding.scale;

			break;
		}
		case Asset::InputBindingSource::MouseWheel:
		{
			if (pMouse)
				return pMouse->GetWheelDelta() * binding.scale;

			break;
		}
		case Asset::InputBindingSource::GamepadButton:
		{
			if (pGamepad && pGamepad->IsButtonDown(static_cast<Input::GamepadButtons>(binding.code)))
				return binding.scale;

			break;
		}
		case Asset::InputBindingSource::GamepadLeftStick:
		{
			if (pGamepad)
				return pGamepad->GetLeftStick() * binding.scale;

			break;
		}
		case Asset::InputBindingSource::GamepadRightStick:
		{
			if (pGamepad)
				return pGamepad->GetRightStick() * binding.scale;

			break;
		}
		case Asset::InputBindingSource::GamepadLeftTrigger:
		{
			if (pGamepad)
				return binding.scale * pGamepad->GetLeftTrigger();

			break;
		}
		case Asset::InputBindingSource::GamepadRightTrigger:
		{
			if (pGamepad)
				return binding.scale * pGamepad->GetRightTrigger();

			break;
		}
		}

		return Math::Vec2f(0.0f, 0.0f);
	}
}
