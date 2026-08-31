#include "ComponentRegistry.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ComponentTypeAttribute.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	ComponentRegistry::~ComponentRegistry()
	{
		Clear();
	}

	ComponentStorage* ComponentRegistry::GetOrCreateStorage(ComponentTypeId typeId)
	{
		// Early exit check
		if (!m_reflection)
		{
			Terminal::Error(StringOps::GetName(this), "No reflection system is bound to this registry");
			return nullptr;
		}

		// Fast iterated check for typeId
		auto it = m_slots.find(typeId);
		if (it != m_slots.end())
			return m_storages[it->second];

		// Get the type
		Reflect::Type* pType = m_reflection->GetType(typeId);
		if (!pType)
			return nullptr;

		return GetOrCreateStorage(*pType);
	}

	ComponentStorage* ComponentRegistry::GetOrCreateStorage(Reflect::Type& type)
	{
		// Fast iterated check for typeId (Its a double check)
		auto it = m_slots.find(type.GetTypeId());
		if (it != m_slots.end())
			return m_storages[it->second];

		// Check if count is smashing MaxComponents
		if (m_storages.GetCount() >= MaxComponents)
		{
			Terminal::Error(StringOps::GetName(this), "Component budget of {} is exhausted", MaxComponents);
			return nullptr;
		}

		// Check for the componentAttribute for fast access 
		if (!type.GetCustomAttribute<ComponentTypeAttribute>())
		{
			Terminal::Error(StringOps::GetName(this), "'{}' carries no ComponentTypeAttribute", type.GetName());
			return nullptr;
		}

		// Create the storage
		auto* pStorage = Memory::Allocator::Create<ComponentStorage>(Memory::CurrLoc(), type);
		if (!pStorage)
		{
			Terminal::Error(StringOps::GetName(this), "Storage for '{}' could not be allocated", type.GetName());
			return nullptr;
		}

		// Emplace properly
		u32 slot = (u32)m_storages.GetCount();
		pStorage->SetSlot(slot);
		m_slots[type.GetTypeId()] = slot;
		m_storages.PushBack(pStorage);

		return pStorage;
	}

	ComponentStorage* ComponentRegistry::FindStorage(ComponentTypeId typeId) const
	{
		// Fast iterated check for typeId
		auto it = m_slots.find(typeId);
		if (it == m_slots.end())
			return nullptr;

		return m_storages[it->second];
	}

	u32 ComponentRegistry::FindSlot(ComponentTypeId typeId) const
	{
		// Fast iterated check for typeId
		auto it = m_slots.find(typeId);
		if (it == m_slots.end())
			return InvalidSlot;

		return it->second;
	}

	void ComponentRegistry::Clear()
	{
		for (ComponentStorage* pStorage : m_storages)
			Memory::Allocator::Delete(pStorage);

		m_storages.Clear();
		m_slots.clear();
	}
}