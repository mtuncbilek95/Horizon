#include "ModuleContext.h"

#include <string_view>
#include <string>

namespace Horizon
{
	EngineReport ModuleContext::OnAttach(Engine*)
	{
		// TODO: Temporary Test for SymbolLibrary
		m_module = Allocator::Create<PAL::SymbolLibrary>(CurrLoc(), PAL::SymbolLibraryDesc());
		if (!m_module)
			return EngineReport("Could not create the symbol library");

		using GenerateFn = void(*)(std::vector<TypeManifest>*);
		auto* GenerateManifests = reinterpret_cast<GenerateFn>(m_module->GetSymbol("GenerateModuleManifestation"));

		if (!GenerateManifests)
			return EngineReport("GenerateModuleManifestation symbol not found");

		GenerateManifests(&m_manifests);

		usize index = 0;
		for (const auto& manifest : m_manifests)
		{
			m_lookup[manifest.GetTypeId()] = index;
			Terminal::Debug(GetName(), "{} has been registered \n\tSize: {} \n\tAbstract: {}, \n\tAttributeCount: {}", manifest.GetName(), 
				manifest.GetSize(), manifest.IsAbstract(), manifest.GetAttributes().size());

			index++;
		}

		Terminal::Info(GetName(), "Loaded {} type manifests", m_manifests.size());
		return EngineReport();
	}

	void ModuleContext::OnDetach()
	{
		m_lookup.clear();
		m_manifests.clear();
		Allocator::Delete(m_module);
	}

	TypeManifest* ModuleContext::GetManifest(ReflectionTypeHandle handl)
	{
		auto it = m_lookup.find(handl);
		if (it == m_lookup.end())
		{
			Terminal::Log(GetName(), "Reflection Handle could not resolve a TypeManifest");
			return nullptr;
		}

		return &m_manifests[it->second];
	}

	std::vector<TypeManifest*> ModuleContext::GetManifestsByBase(ReflectionTypeHandle handl)
	{
		std::vector<TypeManifest*> result;

		for (TypeManifest& manifest : m_manifests)
		{
			if (manifest.GetBaseTypeId() == handl)
				result.push_back(&manifest);
		}

		return result;
	}

	std::vector<TypeManifest*> ModuleContext::GetManifestsByAttribute(ReflectionTypeHandle attrHandle)
	{
		std::vector<TypeManifest*> result;

		for (TypeManifest& manifest : m_manifests)
		{
			for (TypeAttribute* attr : manifest.GetAttributes())
			{
				if (attr->GetTypeId() == attrHandle)
				{
					result.push_back(&manifest);
					break;
				}
			}
		}

		return result;
	}
}