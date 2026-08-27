#include "World.h"

namespace Horizon::Engine
{
	EntityHandle World::CreateEntity()
	{
		EntityHandle entity = m_entities.Create();

		if (entity.IsValid())
			m_signatures[(u32)entity.Index()].reset();

		return entity;
	}

	void World::DestroyEntity(EntityHandle entity)
	{
		if (!m_entities.Destroy(entity))
			return;

		const Signature& signature = m_signatures[(u32)entity.Index()];

		for (ComponentStorage* pStorage : m_components.GetStorages())
		{
			if (signature.test(pStorage->GetSlot()))
				pStorage->Remove(entity);
		}

		m_signatures[(u32)entity.Index()].reset();
	}

	b8 World::EnsureStructural(std::string_view callName) const
	{
		if (!m_structural)
		{
			Terminal::Error(StringOps::GetName(this), "{} was called outside the structural phase", callName);
			return false;
		}

		return true;
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