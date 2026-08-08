#include "ModuleContext.h"

#include <Runtime/Log/Terminal.h>

#include <string_view>
#include <string>

namespace Horizon::Engine
{
	ModuleContext::ModuleContext(PAL::SymbolLibrary* pLibrary) : m_module(pLibrary)
	{
	}

	AppReport ModuleContext::OnAttach(Application* pEngine)
	{
		if (!m_module)
			return AppReport("Could not create the symbol library in Application");

		using GenerateFn = void(*)(List<Reflect::Type>*);
		auto* GenerateManifests = reinterpret_cast<GenerateFn>(m_module->GetSymbol("GenerateModuleManifestation"));

		if (!GenerateManifests)
			return AppReport("GenerateModuleManifestation symbol not found");

		List<Reflect::Type> registery;
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
		return AppReport();
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