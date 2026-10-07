#include "ReflectionSystem.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	ReflectionSystem::ReflectionSystem(Engine* pEngine)
	{
		m_hostLibrary = Memory::Allocator::Create<PAL::SymbolLibrary>(Memory::CurrLoc(), PAL::SymbolLibraryDesc());

		if (!m_hostLibrary)
		{
			pEngine->RequestExit("Host Library has not been created!");
			return;
		}

		if (!RegisterLibrary(m_hostLibrary))
		{
			pEngine->RequestExit("Host manifestation could not be registered");
		}
	}

	ReflectionSystem::~ReflectionSystem()
	{
		if (m_hostLibrary)
		{
			UnregisterLibrary(m_hostLibrary);
		}

		if (!m_libraries.IsEmpty())
		{
			Terminal::Warn(StringOps::GetName(this), "{} libraries were never unregistered, their types die with the system", m_libraries.GetCount());
		}

		Memory::Allocator::Delete(m_hostLibrary);
	}

	b8 ReflectionSystem::RegisterLibrary(const PAL::SymbolLibrary* pLibrary)
	{
		if (!pLibrary)
		{
			Terminal::Error(StringOps::GetName(this), "RegisterLibrary received a null library");
			return false;
		}

		if (FindLibrary(pLibrary))
		{
			Terminal::Warn(StringOps::GetName(this), "{} is already registered", pLibrary->GetName());
			return false;
		}

		using GenerateFn = void(*)(List<Reflect::Type>*);
		auto* GenerateManifests = reinterpret_cast<GenerateFn>(pLibrary->GetSymbol("GenerateModuleManifestation"));

		if (!GenerateManifests)
		{
			Terminal::Error(StringOps::GetName(this), "{} does not export GenerateModuleManifestation", pLibrary->GetName());
			return false;
		}

		List<Reflect::Type> manifests;
		GenerateManifests(&manifests);

		ReflectionLibrary& library = m_libraries.EmplaceBack();
		library.library = pLibrary;
		library.types = std::move(manifests);

		for (Reflect::Type& type : library.types)
		{
			if (m_typeLookup.contains(type.GetTypeId()))
			{
				Terminal::Error(StringOps::GetName(this), "{} from {} collides with an already registered type, skipped", type.GetName(), pLibrary->GetName());
				continue;
			}

			IndexType(type);

			Terminal::Debug(StringOps::GetName(this), "{} has been registered from {}", type.GetName(), pLibrary->GetName().substr(pLibrary->GetName().rfind('/\\') + 1));
		}

		Terminal::Info(StringOps::GetName(this), "{} registered {} type manifests", pLibrary->GetName(), library.types.GetCount());
		return true;
	}

	void ReflectionSystem::UnregisterLibrary(const PAL::SymbolLibrary* pLibrary)
	{
		ReflectionLibrary* pEntry = FindLibrary(pLibrary);

		if (!pEntry)
		{
			Terminal::Warn(StringOps::GetName(this), "UnregisterLibrary could not find the library, nothing to do");
			return;
		}

		for (Reflect::Type& type : pEntry->types)
		{
			auto typeIt = m_typeLookup.find(type.GetTypeId());

			if (typeIt == m_typeLookup.end() || typeIt->second != &type)
			{
				continue;
			}

			UnindexType(type);
		}

		const usize count = pEntry->types.GetCount();
		const usize index = static_cast<usize>(pEntry - m_libraries.GetData());
		m_libraries.RemoveAt(index);

		Terminal::Info(StringOps::GetName(this), "{} unregistered {} type manifests", pLibrary->GetName(), count);
	}

	Reflect::Type* ReflectionSystem::GetType(Reflect::TypeHandle handl)
	{
		auto it = m_typeLookup.find(handl);

		if (it == m_typeLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "Reflect::TypeHandle could not found. I hope you found it xD");
			return nullptr;
		}

		return it->second;
	}

	Reflect::Type* ReflectionSystem::GetTypeByName(const std::string& name)
	{
		auto it = m_nameLookup.find(name);

		if (it == m_nameLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not found in the reflection system.", name);
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

	ReflectionLibrary* ReflectionSystem::FindLibrary(const PAL::SymbolLibrary* pLibrary)
	{
		for (ReflectionLibrary& library : m_libraries)
		{
			if (library.library == pLibrary)
			{
				return &library;
			}
		}

		return nullptr;
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

	void ReflectionSystem::UnindexType(Reflect::Type& type)
	{
		m_typeLookup.erase(type.GetTypeId());
		m_nameLookup.erase(type.GetName());

		auto baseIt = m_byBase.find(type.GetBaseId());

		if (baseIt != m_byBase.end())
		{
			baseIt->second.Remove(&type);

			if (baseIt->second.IsEmpty())
			{
				m_byBase.erase(baseIt);
			}
		}

		for (Reflect::Attribute* pAttr : type.GetAttributes())
		{
			auto attrIt = m_byAttribute.find(pAttr->GetTypeId());

			if (attrIt == m_byAttribute.end())
			{
				continue;
			}

			attrIt->second.Remove(&type);

			if (attrIt->second.IsEmpty())
			{
				m_byAttribute.erase(attrIt);
			}
		}
	}
}