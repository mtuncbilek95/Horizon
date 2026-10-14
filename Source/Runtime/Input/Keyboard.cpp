#include "Keyboard.h"

namespace Horizon::Input
{
	Keyboard::Keyboard(Instance* pInstance, ObjectHandle handle, VendorType vendor, ProductType product)
		: Object(pInstance, handle, DeviceType::Keyboard, vendor, product)
	{
	}

	b8 Keyboard::IsKeyDown(PAL::KeyCode key) const
	{
		if (!IsValidKey(key))
			return false;

		return m_current.test(u64(key));
	}

	b8 Keyboard::IsKeyPressed(PAL::KeyCode key) const
	{
		if (!IsValidKey(key))
			return false;

		return m_current.test(u64(key)) && !m_previous.test(u64(key));
	}

	b8 Keyboard::IsKeyReleased(PAL::KeyCode key) const
	{
		if (!IsValidKey(key))
			return false;

		return !m_current.test(u64(key)) && m_previous.test(u64(key));
	}

	b8 Keyboard::IsAnyKeyDown() const
	{
		return m_current.any();
	}

	void Keyboard::PushState(const KeyboardState& state)
	{
		m_previous = m_current;
		m_current = state;
	}

	b8 Keyboard::IsValidKey(PAL::KeyCode key)
	{
		return key != PAL::KeyCode::Unknown && u64(key) < KeyboardState().size();
	}
}