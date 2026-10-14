#pragma once

#include <Runtime/Input/Object.h>
#include <Runtime/Input/MouseState.h>
#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Input
{
	class RUNTIME_API Mouse final : public Object
	{
	public:
		Mouse(Instance* pInstance, ObjectHandle handle, VendorType vendor, ProductType product);
		~Mouse() final = default;

		b8 IsButtonDown(PAL::MouseButton button) const;
		b8 IsButtonPressed(PAL::MouseButton button) const;
		b8 IsButtonReleased(PAL::MouseButton button) const;

		Math::Vec2f GetDelta() const;
		Math::Vec2f GetWheelDelta() const;

		const MouseState& GetState() const { return m_current; }
		void PushState(const MouseState& state);

	private:
		static b8 HasButton(u32 buttons, PAL::MouseButton button);

	private:
		MouseState m_previous;
		MouseState m_current;
		b8 m_hasState = false;
	};
}