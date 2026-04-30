#pragma once

#include <Runtime/Data/Containers/Math.h>
#include <Runtime/Input/InputDispatcher.h>

namespace Horizon
{
	class Camera
	{
	public:
		Camera(InputDispatcher& dispatcher, f32 moveSpeed = 5.f, f32 sensitivity = 0.1f);

		void Update(f32 deltaTime);

		Math::Mat4f GetViewMatrix() const;
		Math::Mat4f GetProjectionMatrix(f32 fovDeg, f32 aspect, f32 nearPlane, f32 farPlane) const;

		void SetPosition(const Math::Vec3f& pos) { m_position = pos; }
		const Math::Vec3f& GetPosition() const { return m_position; }
		const Math::Vec3f& GetForward() const { return m_front; }

	private:
		void UpdateVectors();
		void RegisterCallbacks(InputDispatcher& dispatcher);

		Math::Vec3f m_position = { 0.f, -2.f, 1.5f };
		Math::Vec3f m_front = { 0.f, 0.f, -1.f };
		Math::Vec3f m_up = { 0.f, 1.f,  0.f };
		Math::Vec3f m_right = { 1.f, 0.f,  0.f };

		f32 m_yaw = 90.f;
		f32 m_pitch = -25.f;
		f32 m_moveSpeed;
		f32 m_sensitivity;

		bool m_keys[512] = {};
		bool m_rightMouseHeld = false;
		bool m_firstMouse = true;
		f64 m_lastMouseX = 0.0;
		f64 m_lastMouseY = 0.0;
	};
}
