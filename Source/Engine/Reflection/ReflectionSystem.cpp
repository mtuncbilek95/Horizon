#include "ReflectionSystem.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <string_view>
#include <string>

namespace Horizon::Engine
{
	ReflectionSystem::ReflectionSystem(Engine* pEngine)
	{
		// Create exec's library to call manifestation.
		m_hostLibrary = Memory::Allocator::Create<PAL::SymbolLibrary>(Memory::CurrLoc(), PAL::SymbolLibraryDesc());
		if (!m_hostLibrary)
		{
			pEngine->RequestExit("Host Library has not been created!");
			return;
		}

		// Functionary
		using GenerateFn = void(*)(List<Reflect::Type>*);
		auto* GenerateManifests = reinterpret_cast<GenerateFn>(m_hostLibrary->GetSymbol("GenerateModuleManifestation"));

		// Check if we have it
		if (!GenerateManifests)
		{
			pEngine->RequestExit("GenerateModuleManifestation symbol not found");
			return;
		}

		// Fill the fuckout
		List<Reflect::Type> registery;
		GenerateManifests(&registery);

		// Registry handl for faster iterations
		usize index = 0;
		for (auto& manifest : registery)
		{
			Terminal::Debug(StringOps::GetName(this), "{} has been registered to reflection system.", manifest.GetName());

			m_lookup[manifest.GetTypeId()] = index;
			m_registeredTypes.PushBack(std::move(manifest));
			index++;
		}

		Terminal::Info(StringOps::GetName(this), "Loaded {} type manifests", m_registeredTypes.GetCount());
	}

	ReflectionSystem::~ReflectionSystem()
	{
		Memory::Allocator::Delete(m_hostLibrary);
	}

	Reflect::Type* ReflectionSystem::GetType(Reflect::TypeHandle handl)
	{
		auto it = m_lookup.find(handl);
		if (it == m_lookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "Reflect::TypeHandle could not found. I hope you found it xD");
			return nullptr;
		}

		return &m_registeredTypes.At(it->second);
	}

	Reflect::Type* ReflectionSystem::GetTypeByName(const std::string& name)
	{
		for (auto& type : m_registeredTypes)
		{
			if (type.GetName() == name)
				return &type;
		}

		return nullptr;
	}

	List<Reflect::Type*> ReflectionSystem::GetTypeByBase(Reflect::TypeHandle handl)
	{
		List<Reflect::Type*> result;

		for (auto& pType : m_registeredTypes)
		{
			if (pType.GetBaseId() == handl)
				result.PushBack(&pType);
		}

		return result;
	}

	List<Reflect::Type*> ReflectionSystem::GetTypeByAttribute(Reflect::TypeHandle attrHandle)
	{
		List<Reflect::Type*> result;

		for (auto& pType : m_registeredTypes)
		{
			for (auto* pAttr : pType.GetAttributes())
			{
				if (pAttr->GetTypeId() == attrHandle)
				{
					result.PushBack(&pType);
					break;
				}
			}
		}

		return result;
	}
}