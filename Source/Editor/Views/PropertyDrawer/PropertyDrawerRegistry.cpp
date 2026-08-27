#include "PropertyDrawerRegistry.h"

#include <Engine/Reflection/ReflectionSystem.h>

namespace Horizon::Editor
{
	PropertyDrawerRegistry::~PropertyDrawerRegistry()
	{
		Clear();
	}

	void PropertyDrawerRegistry::BootstrapDrawers(Engine::Engine* pEngine)
	{
		Clear();

		auto* pModule = pEngine->GetReflectionSystem();

		List<Reflect::Type*> types = pModule->GetTypeByBase(Reflect::TypeOf<PropertyDrawer>());

		for (Reflect::Type* pType : types)
		{
			auto* pDrawer = static_cast<PropertyDrawer*>(pType->Create());

			if (pDrawer == nullptr)
			{
				Terminal::Error(StringOps::GetName(this), "{} type could not be instantiated", pType->GetName());
				continue;
			}

			Reflect::TypeHandle target = pDrawer->GetTargetType();

			if (m_drawers.contains(target))
			{
				Terminal::Warn(StringOps::GetName(this), "{} target type already has a drawer, {} is skipped", target.Index(), pType->GetName());
				Memory::Allocator::Delete(pDrawer);
				continue;
			}

			m_drawers[target] = pDrawer;
		}
	}

	PropertyDrawer* PropertyDrawerRegistry::Find(Reflect::TypeHandle typeId) const
	{
		auto it = m_drawers.find(typeId);

		if (it == m_drawers.end())
			return nullptr;

		return it->second;
	}

	void PropertyDrawerRegistry::Clear()
	{
		for (auto& pair : m_drawers)
			Memory::Allocator::Delete(pair.second);

		m_drawers.clear();
	}
}