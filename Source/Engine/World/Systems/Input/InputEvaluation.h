#pragma once

#include <Engine/Asset/Input/InputAction.h>
#include <Engine/World/System.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetService;
	class InputService;
}

namespace Horizon::Input
{
	class Keyboard;
	class Mouse;
	class Gamepad;
}

namespace Horizon::World::System
{
	HCLASS();
	class InputEvaluation : public Engine::System
	{
		HORIZON_TYPE_REFLECT(InputEvaluation);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;

	private:
		Math::Vec2f ReadBinding(const Asset::InputBinding& binding, Input::Keyboard* pKeyboard, Input::Mouse* pMouse, Input::Gamepad* pGamepad) const;

	private:
		Engine::AssetService* m_assets = nullptr;
		Engine::InputService* m_input = nullptr;
	};
}
