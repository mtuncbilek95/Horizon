#include "Scene.h"

#include <Engine/Asset/Scene/SceneStreamer.h>

namespace Horizon::Asset
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		// TODO: Temporary
		if (m_scene)
			m_scene.Reset();
	}

	void Scene::LoadAsync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Unloaded, Engine::AssetResidency::Pending) != Engine::AssetResidency::Unloaded)
			return;

		GetLoader<SceneStreamer>()->LoadAsync(this);
	}

	void Scene::UnloadAsync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Resident, Engine::AssetResidency::Pending) != Engine::AssetResidency::Resident)
			return;

		GetLoader<SceneStreamer>()->UnloadAsync(this);
	}

	void Scene::LoadSync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Unloaded, Engine::AssetResidency::Pending) != Engine::AssetResidency::Unloaded)
			return;

		GetLoader<SceneStreamer>()->LoadSync(this);
	}

	void Scene::UnloadSync()
	{

	}
}