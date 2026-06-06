#include "SceneLoader.h"

#include <Engine/Asset/AssetRegistry.h>
#include <Engine/Log/Log.h>

#include <Engine/ECS/Components/CameraComp.h>
#include <Engine/ECS/Components/MeshComp.h>
#include <Engine/ECS/Components/TransformComp.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Horizon
{
	EntityHandle LoadScene(const char* modelPath)
	{
		EntityHandle camera = MainWorld().CreateEntity();
		{
			CameraComp cam;
			cam.target = { -8.01f, 1.1f, 0.47f };
			cam.azimuth = -63.0f;
			cam.elevation = -10.5f;
			cam.distance = 0.5f;
			MainWorld().AddComponent(camera, cam);
		}

		const Model& model = AssetSystem().LoadModel(modelPath);
		ConsoleLog().Info("Model: {} instance", model.instances.size());

		for (const ModelInstance& instance : model.instances)
		{
			EntityHandle entity = MainWorld().CreateEntity();

			glm::vec3 scale, translation, skew;
			glm::quat rotation;
			glm::vec4 perspective;
			glm::decompose(instance.transform, scale, rotation, translation, skew, perspective);

			TransformComp transform;
			transform.position = translation;
			transform.rotation = rotation;
			transform.scale = scale;
			MainWorld().AddComponent(entity, transform);

			MeshComp meshComp;
			meshComp.meshId = instance.mesh;
			meshComp.materialId = instance.material;
			MainWorld().AddComponent(entity, meshComp);
		}

		return camera;
	}
}