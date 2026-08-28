#include "World.h"

namespace Horizon::Engine
{
	EntityHandle World::CreateEntity()
	{
		EntityHandle entity = m_entities.Create();

		// Just make sure if we cant create a proper one.
		if (entity.IsValid())
			m_signatures[(u32)entity.Index()].reset();

		return entity;
	}

	void World::DestroyEntity(EntityHandle entity)
	{
		if (!m_entities.Destroy(entity))
			return;

		const Signature& signature = m_signatures[(u32)entity.Index()];

		// After removing entity, erase the components of the related entity
		for (ComponentStorage* pStorage : m_components.GetStorages())
		{
			if (signature.test(pStorage->GetSlot()))
				pStorage->Remove(entity);
		}

		m_signatures[(u32)entity.Index()].reset();
	}

	void World::CollectComponents(EntityHandle entity, List<ComponentObject*>& outComponents)
	{
		outComponents.Clear();

		if (!m_entities.IsAlive(entity))
		{
			Terminal::Warn(StringOps::GetName(this), "CollectComponents on a dead or stale entity");
			return;
		}

		const Signature& signature = m_signatures[(u32)entity.Index()];

		for (ComponentStorage* pStorage : m_components.GetStorages())
		{
			if (!signature.test(pStorage->GetSlot()))
				continue;

			void* pComponent = pStorage->Find(entity);

			if (pComponent)
				outComponents.PushBack(static_cast<ComponentObject*>(pComponent));
		}
	}
}