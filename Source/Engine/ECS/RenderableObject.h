#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace Horizon
{
	class GfxBuffer;

	struct RenderableObject
	{
		glm::mat4 worldMatrix;
		GfxBuffer* indexBuffer;
		u32 vertexBufferView;
		u32 indexCount;
	};

	struct RenderView
	{
		glm::mat4 viewProjection;
		std::vector<RenderableObject> objects;
	};
}