#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Horizon
{
	struct TransformComp
	{
		glm::vec3 position{ 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f };

		glm::mat4 GetMatrix() const
		{
			glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
			matrix *= glm::mat4_cast(rotation);
			matrix = glm::scale(matrix, scale);
			return matrix;
		}

		glm::mat4 GetViewMatrix() const
		{
			glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
			return glm::inverse(translationMatrix * rotationMatrix);
		}
	};
}
