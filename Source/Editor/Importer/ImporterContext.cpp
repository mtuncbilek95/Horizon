#include "ImporterContext.h"

#include <Editor/Importer/ImportTypeAttribute.h>
#include <Engine/Reflection/ReflectionSystem.h>

namespace Horizon::Editor
{
	Engine::ModuleReport ImporterContext::OnInitialize()
	{
		m_reflection = GetEngine()->GetReflectionSystem();

		for (const Reflect::Type* pType : m_reflection->GetTypeByBase(Reflect::TypeOf<AssetImporter>()))
			AddType(pType);

		return Engine::ModuleReport();
	}

	void ImporterContext::OnFinalize()
	{
		for (ImporterEntry& entry : m_importers)
			Memory::Allocator::Delete(entry.pImporter);

		m_importers.Clear();
	}

	void ImporterContext::DeclareDependencies(Engine::ModuleGraph& graph)
	{
	}

	void ImporterContext::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
		{
			if (type.GetBaseId() == Reflect::TypeOf<AssetImporter>())
				AddType(&type);
		}
	}

	void ImporterContext::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			RemoveType(&type);
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

		return it->second;
	}

	AssetImporter* ImporterContext::GetImporter(const std::string& extension) const
	{
		auto it = m_extLookup.find(extension);

		if (it == m_extLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} has no importer", extension);
			return nullptr;
		}

		return it->second;
	}

	b8 ImporterContext::AddType(const Reflect::Type* pType)
	{
		ImportTypeAttribute* pAttr = pType->GetCustomAttribute<ImportTypeAttribute>();

		if (!pAttr)
			return false;

		auto* pImporter = (AssetImporter*)pType->Create();

		if (!pImporter)
		{
			Terminal::Warn(StringOps::GetName(this), "Forgetting to implement a virtual function can cause the previous error!");
			return false;
		}

		for (const std::string& extension : pAttr->GetExtensions())
			m_extLookup[extension] = pImporter;

		m_typeLookup[pAttr->GetType()] = pImporter;
		m_importers.EmplaceBack(pType, pImporter);

		Terminal::Info(StringOps::GetName(this), "{} has been registered", pType->GetName());
		return true;
	}

	void ImporterContext::RemoveType(const Reflect::Type* pType)
	{
		for (usize i = m_importers.GetCount(); i > 0; --i)
		{
			ImporterEntry& entry = m_importers[i - 1];

			if (entry.pType != pType)
				continue;

			std::erase_if(m_extLookup, [&](const auto& pair) { return pair.second == entry.pImporter; });
			std::erase_if(m_typeLookup, [&](const auto& pair) { return pair.second == entry.pImporter; });

			Memory::Allocator::Delete(entry.pImporter);
			m_importers.RemoveAt(i - 1);
		}
	}
}
