#include "ComponentRegistry.h"

namespace Horizon::Engine
{
	ComponentRegistry::~ComponentRegistry()
	{
		for (IComponentStorage* pStorage : m_storages)
			Memory::Allocator::Delete(pStorage);
	}

	IComponentStorage* ComponentRegistry::FindStorage(ComponentTypeId typeId)
	{
		auto it = m_slots.find(typeId);
		if (it == m_slots.end())
		{
			Terminal::Warn(StringOps::GetName(this), "Component type is not registered in this world");
			return nullptr;
		}

		return m_storages[it->second];
	}

	u32 ComponentRegistry::FindSlot(ComponentTypeId typeId) const
	{
		auto it = m_slots.find(typeId);
		if (it == m_slots.end())
			return kInvalid32;

		return it->second;
	}
}