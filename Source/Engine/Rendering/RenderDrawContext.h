#pragma once

#include <Engine/Rendering/RenderableAreaLight.h>
#include <Engine/Rendering/RenderableDirectionalLight.h>
#include <Engine/Rendering/RenderableMesh.h>
#include <Engine/Rendering/RenderablePointLight.h>
#include <Engine/Rendering/RenderableSpotLight.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderDrawContext final
	{
		Math::Mat4f camViewMatrix;
		Math::Mat4f camProjMatrix;
		Math::Mat4f camViewProjMatrix;
		Math::Vec3f camWorldPosition;

		List<RenderableMesh> meshes;
		List<RenderableDirectionalLight> directionalLights;
		List<RenderablePointLight> pointLights;
		List<RenderableSpotLight> spotLights;
		List<RenderableAreaLight> areaLights;
	};
}