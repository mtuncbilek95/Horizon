#include "ReflectionSystem.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	ReflectionSystem::ReflectionSystem(Engine* pEngine)
	{
		PAL::SymbolLibrary mainSymbol;
		const std::string mainName = mainSymbol.GetName().substr(mainSymbol.GetName().find_last_of("/\\") + 1);

		using GenerateFn = void(*)(List<Reflect::Type>*);
		auto* GenerateManifests = reinterpret_cast<GenerateFn>(mainSymbol.GetSymbol("GenerateModuleManifestation"));

		if (!GenerateManifests)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} does not export GenerateModuleManifestation", mainName);
			pEngine->RequestExit("Main manifestation could not be registered");
			return;
		}

		GenerateManifests(&m_types);

		for (Reflect::Type& type : m_types)
		{
			if (m_typeLookup.contains(type.GetTypeId()))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} collides with an already registered type, skipped", type.GetName());
				continue;
			}

			IndexType(type);

			Terminal::Debug(StringHelper::GetTypeName(this), "{} has been registered from {}", type.GetName(), mainName);
		}

		Terminal::Info(StringHelper::GetTypeName(this), "{} registered {} type manifests", mainName, m_types.GetCount());
	}

	Reflect::Type* ReflectionSystem::GetType(Reflect::TypeHandle handl)
	{
		auto it = m_typeLookup.find(handl);

		if (it == m_typeLookup.end())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Reflect::TypeHandle could not found. I hope you found it xD");
			return nullptr;
		}

		return it->second;
	}

	Reflect::Type* ReflectionSystem::GetTypeByName(const std::string& name)
	{
		auto it = m_nameLookup.find(name);

		if (it == m_nameLookup.end())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not found in the reflection system.", name);
			return nullptr;
		}

		return it->second;
	}

	List<Reflect::Type*> ReflectionSystem::GetTypeByBase(Reflect::TypeHandle handl)
	{
		auto it = m_byBase.find(handl);

		if (it == m_byBase.end())
		{
			return {};
		}

		return it->second;
	}

	List<Reflect::Type*> ReflectionSystem::GetTypeByAttribute(Reflect::TypeHandle attrHandle)
	{
		auto it = m_byAttribute.find(attrHandle);

		if (it == m_byAttribute.end())
		{
			return {};
		}

		return it->second;
	}

	void ReflectionSystem::IndexType(Reflect::Type& type)
	{
		m_typeLookup[type.GetTypeId()] = &type;
		m_nameLookup[type.GetName()] = &type;
		m_byBase[type.GetBaseId()].PushBack(&type);

		for (Reflect::Attribute* pAttr : type.GetAttributes())
		{
			m_byAttribute[pAttr->GetTypeId()].PushBack(&type);
		}
	}
}