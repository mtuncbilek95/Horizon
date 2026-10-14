#pragma once

#include <Runtime/Input/Object.h>
#include <Runtime/PAL/Window/KeyCode.h>

#include <bitset>

namespace Horizon::Input
{
	using KeyboardState = std::bitset<512>;

	class RUNTIME_API Keyboard final : public Object
	{
	public:
		Keyboard(Instance* pInstance, ObjectHandle handle, VendorType vendor, ProductType product);
		~Keyboard() final = default;

		b8 IsKeyDown(PAL::KeyCode key) const;
		b8 IsKeyPressed(PAL::KeyCode key) const;
		b8 IsKeyReleased(PAL::KeyCode key) const;
		b8 IsAnyKeyDown() const;

		const KeyboardState& GetState() const { return m_current; }
		void PushState(const KeyboardState& state);

	private:
		static b8 IsValidKey(PAL::KeyCode key);

	private:
		KeyboardState m_previous;
		KeyboardState m_current;
	};
}