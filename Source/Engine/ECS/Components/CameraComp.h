#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Horizon
{
	struct CameraComp
	{
		enum ProjectionType
		{
			CAMERA_PROJ_PERSPECTIVE,
			CAMERA_PROJ_ORTOGRAPHIC
		};

		ProjectionType type = CAMERA_PROJ_PERSPECTIVE;
		f32 fieldOfView = glm::radians(45.f);
		f32 orthoSize = 10.f;

		f32 aspectRatio = 16.f / 9.f;
		f32 nearClip = .1f;
		f32 farClip = 1000.f;

		b8 primary = true;
		b8 fixedAspectRatio = false;

		glm::vec3 target{ 0.0f };
		f32 azimuth = 0.0f;
		f32 elevation = 20.0f;
		f32 distance = 10.0f;

		f32 orbitSpeed = 0.25f;
		f32 panSpeed = 0.005f;
		f32 zoomSpeed = 1.0f;

		f32 minDistance = 0.5f;
		f32 maxDistance = 500.0f;
		f32 minElevation = -89.0f;
		f32 maxElevation = 89.0f;

		glm::mat4 GetProjection() const
		{
			if (type == CAMERA_PROJ_PERSPECTIVE)
			{
				const f32 focalLength = 1.0f / std::tan(fieldOfView * 0.5f);

				glm::mat4 projection(0.0f);
				projection[0][0] = focalLength / aspectRatio;
				projection[1][1] = focalLength;
				projection[2][3] = -1.0f;
				projection[3][2] = nearClip;
				return projection;
			}

			f32 halfWidth = orthoSize * aspectRatio * 0.5f;
			f32 halfHeight = orthoSize * 0.5f;
			return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearClip, farClip);
		}

		glm::vec3 ComputePosition() const
		{
			f32 azimuthRad = glm::radians(azimuth);
			f32 elevationRad = glm::radians(elevation);
			f32 cosElevation = std::cos(elevationRad);
			glm::vec3 direction{
				cosElevation * std::sin(azimuthRad),
				std::sin(elevationRad),
				cosElevation * std::cos(azimuthRad)
			};
			return target + direction * distance;
		}

		glm::mat4 GetView() const
		{
			return glm::lookAt(ComputePosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glm::mat4 GetViewProjection() const
		{
			return GetProjection() * GetView();
		}
	};
}
