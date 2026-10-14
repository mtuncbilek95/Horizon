#include "Gamepad.h"

#include <algorithm>
#include <cmath>

namespace Horizon::Input
{
	namespace
	{
		b8 HasButton(GamepadButtons buttons, GamepadButtons button)
		{
			return (buttons & button) == button;
		}

		Math::Vec2f ApplyRadialDeadzone(f32 x, f32 y, f32 deadzone)
		{
			const f32 length = std::sqrt(x * x + y * y);

			if (length <= deadzone)
				return Math::Vec2f::Zero();

			const f32 scale = (std::min(length, 1.0f) - deadzone) / ((1.0f - deadzone) * length);
			return Math::Vec2f(x * scale, y * scale);
		}

		f32 ApplyTriggerDeadzone(f32 value, f32 deadzone)
		{
			if (value <= deadzone)
				return 0.0f;

			return (value - deadzone) / (1.0f - deadzone);
		}
	}

	Gamepad::Gamepad(Instance* pInstance, ObjectHandle handle, VendorType vendor, ProductType product)
		: Object(pInstance, handle, DeviceType::Gamepad, vendor, product)
	{
	}

	b8 Gamepad::IsButtonDown(GamepadButtons button) const
	{
		return HasButton(m_current.buttons, button);
	}

	b8 Gamepad::IsButtonPressed(GamepadButtons button) const
	{
		return HasButton(m_current.buttons, button) && !HasButton(m_previous.buttons, button);
	}

	b8 Gamepad::IsButtonReleased(GamepadButtons button) const
	{
		return !HasButton(m_current.buttons, button) && HasButton(m_previous.buttons, button);
	}

	Math::Vec2f Gamepad::GetLeftStick() const
	{
		return ApplyRadialDeadzone(m_current.leftX, m_current.leftY, m_stickDeadzone);
	}

	Math::Vec2f Gamepad::GetRightStick() const
	{
		return ApplyRadialDeadzone(m_current.rightX, m_current.rightY, m_stickDeadzone);
	}

	f32 Gamepad::GetLeftTrigger() const
	{
		return ApplyTriggerDeadzone(m_current.leftTrigger, m_triggerDeadzone);
	}

	f32 Gamepad::GetRightTrigger() const
	{
		return ApplyTriggerDeadzone(m_current.rightTrigger, m_triggerDeadzone);
	}

	void Gamepad::PushState(const GamepadState& state)
	{
		m_previous = m_current;
		m_current = state;
	}
}