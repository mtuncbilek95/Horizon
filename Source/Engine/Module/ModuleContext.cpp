#include "ModuleContext.h"

#include <Runtime/Log/Terminal.h>

#include <string_view>
#include <string>

namespace Horizon
{
	ModuleContext::ModuleContext(PAL::SymbolLibrary* pLibrary) : m_module(pLibrary)
	{
	}

	EngineReport ModuleContext::OnAttach(Engine* pEngine)
	{
		if (!m_module)
			return EngineReport("Could not create the symbol library in Engine");

		using GenerateFn = void(*)(std::vector<Reflect::Type>*);
		auto* GenerateManifests = reinterpret_cast<GenerateFn>(m_module->GetSymbol("GenerateModuleManifestation"));

		if (!GenerateManifests)
			return EngineReport("GenerateModuleManifestation symbol not found");

		std::vector<Reflect::Type> registery;
		GenerateManifests(&registery);

		usize index = 0;
		for (auto& manifest : registery)
		{
			Terminal::Debug(GetName(), "{} has been registered \n\tSize: {} \n\tAbstract: {}, \n\tAttributeCount: {} \n\tFieldCount: {}",
				manifest.GetName(), manifest.GetSizeInBytes(), manifest.GetIsAbstract(),
				manifest.GetAttributes().size(), manifest.GetFields().size());

			m_lookup[manifest.GetTypeId()] = index;
			m_registeredTypes.PushBack(std::move(manifest));
			index++;
		}

		Terminal::Info(GetName(), "Loaded {} type manifests", m_registeredTypes.GetCount());
		return EngineReport();
	}

	void ModuleContext::OnDetach()
	{
	}

	Reflect::Type* ModuleContext::GetType(Reflect::TypeHandle handl)
	{
		auto it = m_lookup.find(handl);
		if (it == m_lookup.end())
		{
			Terminal::Error(GetName(), "Reflect::TypeHandle could not found. I hope you found it xD");
			return nullptr;
		}

		return &m_registeredTypes.At(it->second);
	}

	List<Reflect::Type*> ModuleContext::GetTypeByBase(Reflect::TypeHandle handl)
	{
		List<Reflect::Type*> result;

		for (auto& pType : m_registeredTypes)
		{
			if (pType.GetBaseId() == handl)
				result.PushBack(&pType);
		}

		return result;
	}

	List<Horizon::Reflect::Type*> ModuleContext::GetTypeByAttribute(Reflect::TypeHandle attrHandle)
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