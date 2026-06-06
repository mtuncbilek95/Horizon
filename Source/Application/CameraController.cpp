#include "CameraController.h"

#include <Engine/Log/Log.h>
#include <Engine/Window/InputDispatcher.h>
#include <Engine/ECS/Components/CameraComp.h>

#include <glm/glm.hpp>

namespace Horizon
{
	void InstallCameraControls(EntityHandle camera)
	{
		InputSystem().OnMousePressed([camera](const InputMessage& msg)
			{
				auto& cam = MainWorld().GetComponent<CameraComp>(camera);

				switch (msg.type)
				{
				case InputType::MouseMove:
				{
					if (InputSystem().IsMouseButtonDown(MouseButton::Right))
					{
						cam.azimuth += (f32)msg.mouseDX * cam.orbitSpeed;
						cam.elevation -= (f32)msg.mouseDY * cam.orbitSpeed;
						cam.elevation = glm::clamp(cam.elevation, cam.minElevation, cam.maxElevation);
					}
					else if (InputSystem().IsMouseButtonDown(MouseButton::Left))
					{
						glm::vec3 forward = glm::normalize(cam.target - cam.ComputePosition());
						glm::vec3 rightVector = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
						glm::vec3 upVector = glm::cross(rightVector, forward);

						f32 panScale = cam.panSpeed * cam.distance;
						cam.target += (-rightVector * (f32)msg.mouseDX + upVector * (f32)msg.mouseDY) * panScale;
					}
					break;
				}
				case InputType::MouseScroll:
				{
					cam.distance -= (f32)msg.scrollY * cam.zoomSpeed;
					cam.distance = glm::clamp(cam.distance, cam.minDistance, cam.maxDistance);
					break;
				}
				default:
					break;
				}
			});
	}
}