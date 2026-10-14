#include "InputService.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Input/Gamepad.h>
#include <Runtime/Input/Instance.h>
#include <Runtime/Input/Keyboard.h>
#include <Runtime/Input/Mouse.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/Serialization/JsonArchive.h>
#include <Runtime/Serialization/Serializer.h>

#include <cmath>

namespace Horizon::Engine
{
	namespace
	{
		const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle)
		{
			return static_cast<ReflectionSystem*>(pUserData)->GetType(handle);
		}

		const Reflect::Type* ResolveTypeByName(void* pUserData, const std::string& name)
		{
			return static_cast<ReflectionSystem*>(pUserData)->GetTypeByName(name);
		}

		void Accumulate(Math::Vec2f& target, InputAxis axis, f32 value)
		{
			if (axis == InputAxis::X)
				target.X() += value;
			else
				target.Y() += value;
		}

		f32 ReadGamepadAxis(const Input::Gamepad* pGamepad, GamepadAxisCode code)
		{
			switch (code)
			{
			case GamepadAxisCode::LeftX:
				return pGamepad->GetLeftStick().X();
			case GamepadAxisCode::LeftY:
				return pGamepad->GetLeftStick().Y();
			case GamepadAxisCode::RightX:
				return pGamepad->GetRightStick().X();
			case GamepadAxisCode::RightY:
				return pGamepad->GetRightStick().Y();
			case GamepadAxisCode::LeftTrigger:
				return pGamepad->GetLeftTrigger();
			case GamepadAxisCode::RightTrigger:
				return pGamepad->GetRightTrigger();
			default:
				return 0.0f;
			}
		}

		f32 ReadMouseAxis(const Input::Mouse* pMouse, MouseAxisCode code)
		{
			switch (code)
			{
			case MouseAxisCode::DeltaX:
				return pMouse->GetDelta().X();
			case MouseAxisCode::DeltaY:
				return pMouse->GetDelta().Y();
			case MouseAxisCode::WheelX:
				return pMouse->GetWheelDelta().X();
			case MouseAxisCode::WheelY:
				return pMouse->GetWheelDelta().Y();
			default:
				return 0.0f;
			}
		}
	}

	ModuleReport InputService::OnInitialize()
	{
		m_devices = Input::CreateInstance();

		if (!m_devices)
			return ModuleReport("Input backend could not be created");

		Compile();
		return ModuleReport();
	}

	void InputService::OnExecute(const EngineFrame& ctx)
	{
		m_devices->UpdateInputs();

		for (usize i = 0; i < m_compiled.GetCount(); i++)
		{
			const CompiledAction& compiled = m_compiled[i];
			const InputActionMap& map = m_map.maps[compiled.mapIndex];
			InputActionState& state = m_states[i];

			const b8 wasDown = state.down;

			if (map.enabled)
			{
				EvaluateAction(map.actions[compiled.actionIndex], state);
			}
			else
			{
				state.value = Math::Vec2f::Zero();
				state.down = false;
			}

			state.pressed = state.down && !wasDown;
			state.released = !state.down && wasDown;
		}
	}

	void InputService::OnFinalize()
	{
		m_compiled.Clear();
		m_states.Clear();
		m_lookup.clear();

		if (m_devices)
		{
			Memory::Allocator::Delete(m_devices);
			m_devices = nullptr;
		}
	}

	void InputService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Precedes<WorldService>();
	}

	void InputService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void InputService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}

	b8 InputService::LoadInputFromFile(const std::string& filePath)
	{
		if (!PAL::File::Exists(filePath))
		{
			Terminal::Warn(StringOps::GetName(this), "{} does not exist, keeping the current input map", filePath);
			return false;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(filePath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		std::string text;
		const b8 wasRead = PAL::File::ReadString(request, text);
		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be read", filePath);
			return false;
		}

		ReflectionSystem* pReflection = GetEngine()->GetReflectionSystem();
		const Reflect::Type* pType = pReflection->GetType(Reflect::TypeOf<InputMap>());

		if (!pType)
		{
			Terminal::Error(StringOps::GetName(this), "InputMap is not registered in reflection");
			return false;
		}

		InputMap loaded;
		JsonArchiveReader reader(text);
		Serializer serializer(pReflection, ResolveType, ResolveTypeByName);
		serializer.Deserialize(&loaded, *pType, reader);

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} has invalid input map data", filePath);
			return false;
		}

		SetMap(loaded);
		Terminal::Info(StringOps::GetName(this), "Input map loaded from {} with {} actions", filePath, m_compiled.GetCount());
		return true;
	}

	b8 InputService::SaveInputToFile(const std::string& filePath) const
	{
		ReflectionSystem* pReflection = GetEngine()->GetReflectionSystem();
		const Reflect::Type* pType = pReflection->GetType(Reflect::TypeOf<InputMap>());

		if (!pType)
		{
			Terminal::Error(StringOps::GetName(this), "InputMap is not registered in reflection");
			return false;
		}

		JsonArchiveWriter writer;
		Serializer serializer(pReflection, ResolveType, ResolveTypeByName);
		serializer.Serialize(&m_map, *pType, writer);

		if (!PAL::File::Exists(filePath) && !PAL::File::Create(filePath))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be created", filePath);
			return false;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(filePath, PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);

		const std::string text = writer.ToString();
		const b8 wasWritten = PAL::File::Truncate(request, 0) && PAL::File::WriteString(request, text);
		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be written", filePath);
			return false;
		}

		return true;
	}

	void InputService::SetMap(const InputMap& map)
	{
		m_map = map;
		Compile();
	}

	b8 InputService::SetMapEnabled(std::string_view mapName, b8 enabled)
	{
		for (InputActionMap& map : m_map.maps)
		{
			if (map.name != mapName)
				continue;

			map.enabled = enabled;
			return true;
		}

		Terminal::Warn(StringOps::GetName(this), "{} is not a known action map", mapName);
		return false;
	}

	InputActionHandle InputService::FindAction(std::string_view name) const
	{
		return FindAction(InputAction::HashName(name));
	}

	InputActionHandle InputService::FindAction(u64 nameHash) const
	{
		auto it = m_lookup.find(nameHash);

		if (it == m_lookup.end())
		{
			Terminal::Warn(StringOps::GetName(this), "No input action with hash {:#x}", nameHash);
			return InvalidInputAction;
		}

		return it->second;
	}

	b8 InputService::IsDown(InputActionHandle handle) const
	{
		return IsValid(handle) && m_states[handle].down;
	}

	b8 InputService::WasPressed(InputActionHandle handle) const
	{
		return IsValid(handle) && m_states[handle].pressed;
	}

	b8 InputService::WasReleased(InputActionHandle handle) const
	{
		return IsValid(handle) && m_states[handle].released;
	}

	f32 InputService::ReadAxis(InputActionHandle handle) const
	{
		if (!IsValid(handle))
			return 0.0f;

		return m_states[handle].value.X();
	}

	Math::Vec2f InputService::ReadVector2(InputActionHandle handle) const
	{
		if (!IsValid(handle))
			return Math::Vec2f::Zero();

		return m_states[handle].value;
	}

	void InputService::Compile()
	{
		m_compiled.Clear();
		m_states.Clear();
		m_lookup.clear();

		for (usize mapIndex = 0; mapIndex < m_map.maps.GetCount(); mapIndex++)
		{
			const InputActionMap& map = m_map.maps[mapIndex];

			for (usize actionIndex = 0; actionIndex < map.actions.GetCount(); actionIndex++)
			{
				const InputAction& action = map.actions[actionIndex];
				const u64 hash = InputAction::HashName(action.name);

				if (m_lookup.contains(hash))
				{
					Terminal::Warn(StringOps::GetName(this), "Input action '{}' is defined more than once, the later one is ignored", action.name);
					continue;
				}

				CompiledAction compiled = {};
				compiled.nameHash = hash;
				compiled.mapIndex = u32(mapIndex);
				compiled.actionIndex = u32(actionIndex);

				m_lookup[hash] = InputActionHandle(m_compiled.GetCount());
				m_compiled.PushBack(compiled);
				m_states.PushBack({});
			}
		}
	}

	void InputService::EvaluateAction(const InputAction& action, InputActionState& state) const
	{
		Math::Vec2f value = Math::Vec2f::Zero();
		b8 anyButtonDown = false;

		for (const InputBinding& binding : action.bindings)
		{
			b8 buttonDown = false;
			const Math::Vec2f contribution = EvaluateBinding(binding, buttonDown);

			value = value + contribution;
			anyButtonDown = anyButtonDown || buttonDown;
		}

		if (action.type == InputActionType::Axis2D && action.normalize)
		{
			const f32 length = std::sqrt(value.X() * value.X() + value.Y() * value.Y());

			if (length > 1.0f)
				value = Math::Vec2f(value.X() / length, value.Y() / length);
		}

		if (action.type == InputActionType::Axis1D && action.normalize)
		{
			if (value.X() > 1.0f)
				value.X() = 1.0f;

			if (value.X() < -1.0f)
				value.X() = -1.0f;
		}

		const f32 magnitude = std::sqrt(value.X() * value.X() + value.Y() * value.Y());

		state.value = value;
		state.down = anyButtonDown || magnitude >= action.pressThreshold;
	}

	Math::Vec2f InputService::EvaluateBinding(const InputBinding& binding, b8& outButtonDown) const
	{
		Math::Vec2f result = Math::Vec2f::Zero();
		outButtonDown = false;

		switch (binding.source)
		{
		case InputSource::Key:
		{
			const usize count = m_devices->GetObjectCount(Input::DeviceType::Keyboard);

			for (usize i = 0; i < count; i++)
			{
				if (m_devices->GetKeyboard(i)->IsKeyDown(PAL::KeyCode(binding.code)))
					outButtonDown = true;
			}

			if (outButtonDown)
				Accumulate(result, binding.axis, binding.scale);

			break;
		}
		case InputSource::MouseButton:
		{
			const usize count = m_devices->GetObjectCount(Input::DeviceType::Mouse);

			for (usize i = 0; i < count; i++)
			{
				if (m_devices->GetMouse(i)->IsButtonDown(PAL::MouseButton(binding.code)))
					outButtonDown = true;
			}

			if (outButtonDown)
				Accumulate(result, binding.axis, binding.scale);

			break;
		}
		case InputSource::MouseAxis:
		{
			const usize count = m_devices->GetObjectCount(Input::DeviceType::Mouse);
			f32 total = 0.0f;

			for (usize i = 0; i < count; i++)
				total += ReadMouseAxis(m_devices->GetMouse(i), MouseAxisCode(binding.code));

			Accumulate(result, binding.axis, total * binding.scale);
			break;
		}
		case InputSource::GamepadButton:
		{
			if (binding.playerIndex >= m_devices->GetObjectCount(Input::DeviceType::Gamepad))
				break;

			const Input::Gamepad* pGamepad = m_devices->GetGamepad(binding.playerIndex);

			if (pGamepad->IsButtonDown(Input::GamepadButtons(binding.code)))
			{
				outButtonDown = true;
				Accumulate(result, binding.axis, binding.scale);
			}

			break;
		}
		case InputSource::GamepadAxis:
		{
			if (binding.playerIndex >= m_devices->GetObjectCount(Input::DeviceType::Gamepad))
				break;

			const Input::Gamepad* pGamepad = m_devices->GetGamepad(binding.playerIndex);
			Accumulate(result, binding.axis, ReadGamepadAxis(pGamepad, GamepadAxisCode(binding.code)) * binding.scale);
			break;
		}
		default:
			break;
		}

		return result;
	}
}