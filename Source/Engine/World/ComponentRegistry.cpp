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
		for (ComponentStorage* pStorage : m_storages)
			Memory::Allocator::Delete(pStorage);
	}

	ComponentStorage* ComponentRegistry::GetOrCreateStorage(ComponentTypeId typeId)
	{
		auto it = m_slots.find(typeId);
		if (it != m_slots.end())
			return m_storages[it->second];

		if (!m_reflection)
		{
			Terminal::Error(StringOps::GetName(this), "No reflection system is bound to this registry");
			return nullptr;
		}

		Reflect::Type* pType = m_reflection->GetType(typeId);
		if (!pType)
			return nullptr;

		return GetOrCreateStorage(*pType);
	}

	ComponentStorage* ComponentRegistry::GetOrCreateStorage(Reflect::Type& type)
	{
		auto it = m_slots.find(type.GetTypeId());
		if (it != m_slots.end())
			return m_storages[it->second];

		if (m_storages.GetCount() >= MaxComponents)
		{
			Terminal::Error(StringOps::GetName(this), "Component budget of {} is exhausted", MaxComponents);
			return nullptr;
		}

		if (!type.GetCustomAttribute<ComponentTypeAttribute>())
		{
			Terminal::Error(StringOps::GetName(this), "'{}' carries no ComponentTypeAttribute", type.GetName());
			return nullptr;
		}

		auto* pStorage = Memory::Allocator::Create<ComponentStorage>(Memory::CurrLoc(), type);
		if (!pStorage)
		{
			Terminal::Error(StringOps::GetName(this), "Storage for '{}' could not be allocated", type.GetName());
			return nullptr;
		}

		u32 slot = (u32)m_storages.GetCount();
		pStorage->SetSlot(slot);
		m_slots[type.GetTypeId()] = slot;
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
			return InvalidSlot;

		return it->second;
	}
}