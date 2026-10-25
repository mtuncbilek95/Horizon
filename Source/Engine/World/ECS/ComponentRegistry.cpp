#include "ComponentRegistry.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	ComponentRegistry::~ComponentRegistry()
	{
		Clear();
	}

	ComponentStorage* ComponentRegistry::GetOrCreateStorage(const Reflect::Type* pType)
	{
		if (!pType)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Cannot create a storage for a null type");
			return nullptr;
		}

		auto it = m_slots.find(pType->GetTypeId());
		if (it != m_slots.end())
			return m_storages[it->second];

		if (m_storages.GetCount() >= MaxComponents)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Component budget of {} is exhausted", MaxComponents);
			Terminal::Warn("Developer Info", "How the fuck did you need more than 256 component slots?? Re-assess your design god damn it.");
			return nullptr;
		}

		auto* pStorage = new ComponentStorage(pType);
		if (!pStorage)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Storage for '{}' could not be allocated", pType->GetName());
			return nullptr;
		}

		const u32 slot = (u32)m_storages.GetCount();
		pStorage->SetSlot(slot);
		m_slots[pType->GetTypeId()] = slot;
		m_storages.PushBack(pStorage);

		return pStorage;
	}

	ComponentStorage* ComponentRegistry::FindStorage(ComponentTypeId typeId) const
	{
		auto it = m_slots.find(typeId);
		if (it == m_slots.end())
			return nullptr;

		return m_storages[it->second];
	}

	u32 ComponentRegistry::FindSlot(ComponentTypeId typeId) const
	{
		auto it = m_slots.find(typeId);
		if (it == m_slots.end())
			return kInvalid32;

		return it->second;
	}

	void ComponentRegistry::Clear()
	{
		for (ComponentStorage* pStorage : m_storages)
			delete pStorage;

		m_storages.Clear();
		m_slots.clear();
	}
}