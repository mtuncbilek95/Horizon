#include "Scene.h"

namespace Horizon::Engine
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	EntityHandle Scene::AddEntity()
	{
		return m_entities.Create();
	}

	void Scene::RemoveEntity(EntityHandle handl)
	{
		if (!m_entities.IsAlive(handl))
			return;

		const Signature* pSignature = m_entities.GetSignatureOf(handl);
		for (auto* pStorage : m_components.GetStorages())
		{
			if (pSignature->test(pStorage->GetSlot()))
				pStorage->Remove(handl);
		}

		m_entities.Destroy(handl);
	}

	b8 Scene::IsAlive(EntityHandle handl) const
	{
		return m_entities.IsAlive(handl);
	}
}