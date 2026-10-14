#include "Mouse.h"

namespace Horizon::Input
{
	Mouse::Mouse(Instance* pInstance, ObjectHandle handle, VendorType vendor, ProductType product)
		: Object(pInstance, handle, DeviceType::Mouse, vendor, product)
	{
	}

	b8 Mouse::IsButtonDown(PAL::MouseButton button) const
	{
		return HasButton(m_current.buttons, button);
	}

	b8 Mouse::IsButtonPressed(PAL::MouseButton button) const
	{
		return HasButton(m_current.buttons, button) && !HasButton(m_previous.buttons, button);
	}

	b8 Mouse::IsButtonReleased(PAL::MouseButton button) const
	{
		return !HasButton(m_current.buttons, button) && HasButton(m_previous.buttons, button);
	}

	Math::Vec2f Mouse::GetDelta() const
	{
		return Math::Vec2f(f32(m_current.positionX - m_previous.positionX), f32(m_current.positionY - m_previous.positionY));
	}

	Math::Vec2f Mouse::GetWheelDelta() const
	{
		return Math::Vec2f(f32(m_current.wheelX - m_previous.wheelX), f32(m_current.wheelY - m_previous.wheelY));
	}

	void Mouse::PushState(const MouseState& state)
	{
		m_previous = m_hasState ? m_current : state;
		m_current = state;
		m_hasState = true;
	}

	b8 Mouse::HasButton(u32 buttons, PAL::MouseButton button)
	{
		return (buttons & (1u << u32(button))) != 0;
	}
}