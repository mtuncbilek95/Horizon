#include "ImporterContext.h"

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Engine/Reflection/ReflectionSystem.h>

namespace Horizon::Editor
{
	Engine::ModuleReport ImporterContext::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();
		List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<AssetImporter>());

		for (auto* pType : types)
		{
			ImportTypeAttribute* pAttr = pType->GetCustomAttribute<ImportTypeAttribute>();
			if (!pAttr)
				continue;

			m_extLookup[pAttr->GetExtensions().At(0)] = m_assets.GetCount();
			m_typeLookup[pAttr->GetType()] = m_assets.GetCount();
			m_assets.PushBack((AssetImporter*)pType->Create());

			Terminal::Info(StringOps::GetName(this), "{} has been registered", pType->GetName());
		}

		return Engine::ModuleReport();
	}

	void ImporterContext::OnFinalize()
	{
		for (auto* pObj : m_assets)
			Memory::Allocator::Delete(pObj);
	}

	void ImporterContext::DeclareDependencies(Engine::ModuleGraph& graph)
	{
	}

	AssetImporter* ImporterContext::GetImporter(Reflect::TypeHandle handl) const
	{
		auto it = m_typeLookup.find(handl);
		if (it == m_typeLookup.end())
		{
			Reflect::Type* pType = m_reflection->GetType(handl);
			std::string err = !pType ? "The file that has been drag-dropped has no asset" : std::format("{} has no importer", pType->GetName());
			Terminal::Error(StringOps::GetName(this), "{}", err);
			return nullptr;
		}

		return m_assets[it->second];
	}

	AssetImporter* ImporterContext::GetImporter(const std::string& extension) const
	{
		auto it = m_extLookup.find(extension);
		if (it == m_extLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} has no importer", extension);
			return nullptr;
		}

		return m_assets[it->second];
	}
}