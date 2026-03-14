//#include "PlayerCharacter.h"
//
//namespace Horizon
//{
//	PlayerCharacter::PlayerCharacter()
//	{
//		AddComponent<TransformComponent>();
//		AddComponent<CameraComponent>();
//		AddComponent<InputComponent>();
//	}
//
//	PlayerCharacter::~PlayerCharacter()
//	{
//	}
//
//	void PlayerCharacter::OnStart()
//	{
//		auto& cam = GetComponent<CameraComponent>();
//		cam.SetCameraType(CameraType::Perspective);
//		cam.SetFOV(45.f);
//
//		auto& input = GetComponent<InputComponent>();
//
//		input.BindMouseAction([this](const InputMessage& msg)
//			{
//				m_yaw += msg.mouse.deltaX * m_mouseSensitivity;
//				m_pitch = std::clamp(m_pitch + msg.mouse.deltaY * m_mouseSensitivity, -89.f, 89.f);
//
//				auto& transform = GetComponent<TransformComponent>();
//				transform.rotation = Math::Quaternion::FromEuler(m_pitch, m_yaw, 0.f);
//			});
//
//		input.BindKeyAction([this](const InputMessage& msg)
//			{
//				auto& transform = GetComponent<TransformComponent>();
//
//				Math::Vec3f move = { 0.f, 0.f, 0.f };
//
//				if (msg.key.keyCode == Key::W) move += transform.GetForward();
//				if (msg.key.keyCode == Key::S) move -= transform.GetForward();
//				if (msg.key.keyCode == Key::A) move -= transform.GetRight();
//				if (msg.key.keyCode == Key::D) move += transform.GetRight();
//
//				transform.position += move * m_moveSpeed;
//			});
//	}
//
//	void PlayerCharacter::OnUpdate()
//	{
//	}
//
//	void PlayerCharacter::OnStop()
//	{
//	}
//}