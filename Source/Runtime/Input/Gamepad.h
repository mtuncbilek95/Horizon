#pragma once

#include <Runtime/Input/Object.h>
#include <Runtime/Input/GamepadState.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Input
{
	class RUNTIME_API Gamepad final : public Object
	{
	public:
		Gamepad(Instance* pInstance, ObjectHandle handle, VendorType vendor, ProductType product);
		~Gamepad() final = default;

		b8 IsButtonDown(GamepadButtons button) const;
		b8 IsButtonPressed(GamepadButtons button) const;
		b8 IsButtonReleased(GamepadButtons button) const;

		Math::Vec2f GetLeftStick() const;
		Math::Vec2f GetRightStick() const;
		f32 GetLeftTrigger() const;
		f32 GetRightTrigger() const;

		void SetStickDeadzone(f32 deadzone) { m_stickDeadzone = deadzone; }
		void SetTriggerDeadzone(f32 deadzone) { m_triggerDeadzone = deadzone; }

		const GamepadState& GetState() const { return m_current; }
		void PushState(const GamepadState& state);

	private:
		GamepadState m_previous;
		GamepadState m_current;
		f32 m_stickDeadzone = 0.15f;
		f32 m_triggerDeadzone = 0.05f;
	};
}