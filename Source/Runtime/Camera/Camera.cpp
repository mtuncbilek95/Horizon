#include <Runtime/Camera/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Horizon
{
	Camera::Camera(InputDispatcher& dispatcher, f32 moveSpeed, f32 sensitivity)
		: m_moveSpeed(moveSpeed), m_sensitivity(sensitivity)
	{
		RegisterCallbacks(dispatcher);
		UpdateVectors();
	}

	void Camera::RegisterCallbacks(InputDispatcher& dispatcher)
	{
		dispatcher.OnKeyPressed([this](const InputMessage& msg)
			{
				if (msg.type != InputType::Key)
					return;

				bool down = msg.keyAction == InputAction::Press || msg.keyAction == InputAction::Repeat;
				i32 code = static_cast<i32>(msg.key);
				if (code >= 0 && code < 512)
					m_keys[code] = down;
			});

		dispatcher.OnMousePressed([this](const InputMessage& msg)
			{
				if (msg.type == InputType::MouseButton && msg.button == MouseButton::Right)
				{
					m_rightMouseHeld = msg.mouseAction == InputAction::Press;
					m_firstMouse = true;
					return;
				}

				if (msg.type == InputType::MouseMove && m_rightMouseHeld)
				{
					if (m_firstMouse)
					{
						m_lastMouseX = msg.mouseX;
						m_lastMouseY = msg.mouseY;
						m_firstMouse = false;
						return;
					}

					f32 dx = static_cast<f32>(msg.mouseX - m_lastMouseX) * m_sensitivity;
					f32 dy = static_cast<f32>(m_lastMouseY - msg.mouseY) * m_sensitivity;
					m_lastMouseX = msg.mouseX;
					m_lastMouseY = msg.mouseY;

					m_yaw += dx;
					m_pitch = glm::clamp(m_pitch + dy, -89.f, 89.f);
					UpdateVectors();
				}
			});
	}

	void Camera::Update(f32 deltaTime)
	{
		f32 speed = m_moveSpeed * deltaTime;

		if (m_keys[static_cast<i32>(KeyCode::LeftShift)] || m_keys[static_cast<i32>(KeyCode::RightShift)])
			speed *= 3.f;

		if (m_keys[static_cast<i32>(KeyCode::W)]) m_position += m_front * speed;
		if (m_keys[static_cast<i32>(KeyCode::S)]) m_position -= m_front * speed;
		if (m_keys[static_cast<i32>(KeyCode::A)]) m_position -= m_right * speed;
		if (m_keys[static_cast<i32>(KeyCode::D)]) m_position += m_right * speed;
		if (m_keys[static_cast<i32>(KeyCode::E)]) m_position += m_up * speed;
		if (m_keys[static_cast<i32>(KeyCode::Q)]) m_position -= m_up * speed;
	}

	Math::Mat4f Camera::GetViewMatrix() const
	{
		return glm::lookAt(m_position, m_position + m_front, m_up);
	}

	Math::Mat4f Camera::GetProjectionMatrix(f32 fovDeg, f32 aspect, f32 nearPlane, f32 farPlane) const
	{
		auto proj = glm::perspective(glm::radians(fovDeg), aspect, nearPlane, farPlane);
		proj[1][1] *= -1.f;
		return proj;
	}

	void Camera::UpdateVectors()
	{
		Math::Vec3f front;
		front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
		front.y = glm::sin(glm::radians(m_pitch));
		front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));

		m_front = glm::normalize(front);
		m_right = glm::normalize(glm::cross(m_front, { 0.f, 1.f, 0.f }));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}
}
