#include "Scene.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	Scene::Scene(ReflectionSystem* pReflection) : m_reflection(pReflection)
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
		for (ComponentStorage* pStorage : m_components.GetStorages())
		{
			if (pSignature->test(pStorage->GetSlot()))
				pStorage->Remove(handl);
		}

		m_entities.Destroy(handl);
	}

	void Scene::RemoveEntity(usize index)
	{
		EntityHandle handl = m_entities.GetHandleAt(u32(index));
		RemoveEntity(handl);
	}

	b8 Scene::IsAlive(EntityHandle handl) const
	{
		return m_entities.IsAlive(handl);
	}

	ComponentObject* Scene::AddComponent(EntityHandle handl, ComponentTypeId typeId)
	{
		ComponentStorage* pStorage = ResolveStorage(typeId);
		if (!pStorage)
			return nullptr;

		return AddComponentTo(pStorage, handl, nullptr);
	}

	void Scene::RemoveComponent(EntityHandle handl, ComponentTypeId typeId)
	{
		if (!m_entities.IsAlive(handl))
			return;

		ComponentStorage* pStorage = m_components.FindStorage(typeId);
		if (!pStorage)
			return;

		pStorage->Remove(handl);

		if (Signature* pSignature = m_entities.GetSignatureOf(handl))
			pSignature->reset(pStorage->GetSlot());
	}

	ComponentObject* Scene::FindComponent(EntityHandle handl, ComponentTypeId typeId) const
	{
		if (!m_entities.IsAlive(handl))
			return nullptr;

		ComponentStorage* pStorage = m_components.FindStorage(typeId);
		if (!pStorage)
			return nullptr;

		return pStorage->Find(handl);
	}

	b8 Scene::HasComponent(EntityHandle handl, ComponentTypeId typeId) const
	{
		if (!m_entities.IsAlive(handl))
			return false;

		const u32 slot = m_components.FindSlot(typeId);
		if (slot == kInvalid32)
			return false;

		return m_entities.GetSignatureOf(handl)->test(slot);
	}

	ComponentStorage* Scene::ResolveStorage(ComponentTypeId typeId)
	{
		if (ComponentStorage* pStorage = m_components.FindStorage(typeId))
			return pStorage;

		const Reflect::Type* pType = m_reflection->GetType(typeId);
		if (!pType)
		{
			Terminal::Error(StringOps::GetName(this), "Component type is not reflected. Did you forget HCLASS or to regenerate?");
			return nullptr;
		}

		return m_components.GetOrCreateStorage(pType);
	}

	ComponentObject* Scene::AddComponentTo(ComponentStorage* pStorage, EntityHandle handl, void* pSource)
	{
		if (!m_entities.IsAlive(handl))
		{
			Terminal::Warn(StringOps::GetName(this), "AddComponent on a dead or stale entity");
			return nullptr;
		}

		ComponentObject* pComponent = pSource ? pStorage->Insert(handl, pSource) : pStorage->InsertBlind(handl);
		if (!pComponent)
			return nullptr;

		m_entities.GetSignatureOf(handl)->set(pStorage->GetSlot());

		return pComponent;
	}
}