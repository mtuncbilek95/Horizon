#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Input/InputMap.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Math/Vec2f.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace Horizon::Input
{
	class Instance;
	class Keyboard;
	class Mouse;
	class Gamepad;
}

namespace Horizon::Engine
{
	using InputActionHandle = u32;
	inline constexpr InputActionHandle InvalidInputAction = kInvalid32;

	struct InputActionState
	{
		Math::Vec2f value = Math::Vec2f::Zero();
		b8 down = false;
		b8 pressed = false;
		b8 released = false;
	};

	class ENGINE_API InputService final : public Service
	{
		struct CompiledAction
		{
			u64 nameHash = 0;
			u32 mapIndex = 0;
			u32 actionIndex = 0;
		};

	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

		b8 LoadInputFromFile(const std::string& filePath);
		b8 SaveInputToFile(const std::string& filePath) const;

		const InputMap& GetMap() const { return m_map; }
		void SetMap(const InputMap& map);

		b8 SetMapEnabled(std::string_view mapName, b8 enabled);

		InputActionHandle FindAction(std::string_view name) const;
		InputActionHandle FindAction(u64 nameHash) const;

		b8 IsDown(InputActionHandle handle) const;
		b8 WasPressed(InputActionHandle handle) const;
		b8 WasReleased(InputActionHandle handle) const;
		f32 ReadAxis(InputActionHandle handle) const;
		Math::Vec2f ReadVector2(InputActionHandle handle) const;

		Input::Instance* GetDevices() const { return m_devices; }

	private:
		void Compile();
		void EvaluateAction(const InputAction& action, InputActionState& state) const;
		Math::Vec2f EvaluateBinding(const InputBinding& binding, b8& outButtonDown) const;

		b8 IsValid(InputActionHandle handle) const { return handle < m_states.GetCount(); }

	private:
		Input::Instance* m_devices = nullptr;

		InputMap m_map;
		List<CompiledAction> m_compiled;
		List<InputActionState> m_states;
		std::unordered_map<u64, InputActionHandle> m_lookup;
	};
}