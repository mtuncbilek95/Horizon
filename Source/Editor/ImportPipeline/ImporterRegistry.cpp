#include "ImporterRegistry.h"

#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>

namespace Horizon::Editor
{
	ImporterRegistry::~ImporterRegistry()
	{
		for (const auto& entry : m_pipelines)
			Memory::Allocator::Delete(entry.pPipeline);

		for (const auto& entry : m_importers)
			Memory::Allocator::Delete(entry.pImporter);

		m_pipelines.Clear();
		m_pipelineLookup.clear();
		m_typeToPipelineLookup.clear();

		m_importers.Clear();
		m_typeToImporterLookup.clear();
	}

	void ImporterRegistry::BootstrapViews(Engine::Engine* ctx)
	{
		m_engine = ctx;

		auto* pReflection = ctx->GetReflectionSystem();

		// Register the Import Pipeline processor mechs
		for (auto* pType : pReflection->GetTypeByBase(Reflect::TypeOf<ImportPipeline>()))
			AddPipelineType(pType);

		// Register the importers such as ufbx/fast-gltf/ktx etc
		for (auto* pType : pReflection->GetTypeByBase(Reflect::TypeOf<AssetImporter>()))
			AddImporterType(pType);
	}

	void ImporterRegistry::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		// Check for pipelines first
		for (const Reflect::Type& type : library.types)
			AddPipelineType(&type);

		// Check for importers second
		for (const Reflect::Type& type : library.types)
			AddImporterType(&type);
	}

	void ImporterRegistry::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		// Check for pipelines first
		for (const Reflect::Type& type : library.types)
			RemovePipelineType(&type);

		// Check for importers second
		for (const Reflect::Type& type : library.types)
			RemoveImporterType(&type);
	}

	ImportPipeline* ImporterRegistry::GetPipeline(Reflect::TypeHandle handl)
	{
		for (const auto& entry : m_pipelines)
		{
			if (entry.pType->GetTypeId() == handl)
				return entry.pPipeline;
		}

		Reflect::Type* pType = m_engine->GetReflectionSystem()->GetType(handl);
		if (!pType)
		{
			Terminal::Warn(StringOps::GetName(this), "Type has not registered to ReflectionSystem.");
			return nullptr;
		}

		Terminal::Warn(StringOps::GetName(this), "Type '{}' has not registered to ImporterRegistry", pType->GetName());
		return nullptr;
	}

	ImportPipeline* ImporterRegistry::GetPipeline(const std::string extension)
	{
		auto it = m_pipelineLookup.find(extension);
		if (it == m_pipelineLookup.end())
		{
			Terminal::Warn(StringOps::GetName(this), "Extension '{}' has no default importer pipeline system. Could not have the asset as well check properly", extension);
			return nullptr;
		}

		return m_pipelines[it->second].pPipeline;
	}

	b8 ImporterRegistry::AddPipelineType(const Reflect::Type* pType)
	{
		if (pType->GetBaseId() != Reflect::TypeOf<ImportPipeline>())
			return false;

		auto* pAttr = pType->GetCustomAttribute<ImportPipelineInfoAttribute>();
		if (!pAttr)
		{
			Terminal::Error(StringOps::GetName(this), "{} needs ImportPipelineInfoAttribute to be added. Skipping it..", pType->GetName());
			return false;
		}

		auto* pObject = (ImportPipeline*)pType->Create();
		if (!pObject)
		{
			Terminal::Error(StringOps::GetName(this), "{} has virtual function issues. You won't have this system.", pType->GetName());
			return false;
		}

		PipelineEntry entry = {};
		entry.pPipeline = pObject;
		entry.pType = pType;
		entry.extensions = pAttr->GetExtensions();

		m_typeToPipelineLookup[pAttr->GetWorkingAssetType()] = m_pipelines.GetCount();
		for (const auto& ext : pAttr->GetExtensions())
			m_pipelineLookup[ext] = m_pipelines.GetCount();

		m_pipelines.PushBack(entry);
		Terminal::Info(StringOps::GetName(this), "{} has been registered", pType->GetName());

		return true;
	}

	b8 ImporterRegistry::RemovePipelineType(const Reflect::Type* pType)
	{
		if (pType->GetBaseId() != Reflect::TypeOf<ImportPipeline>())
			return false;

		usize removedIndex = m_pipelines.GetCount();
		for (usize i = 0; i < m_pipelines.GetCount(); ++i)
		{
			if (m_pipelines[i].pType == pType)
			{
				removedIndex = i;
				break;
			}
		}

		if (removedIndex == m_pipelines.GetCount())
		{
			Terminal::Warn(StringOps::GetName(this), "{} is not registered to ImporterRegistry", pType->GetName());
			return false;
		}

		Memory::Allocator::Delete(m_pipelines[removedIndex].pPipeline);
		m_pipelines.RemoveAt(removedIndex);

		// Clear the ext lookup as well
		for (auto it = m_pipelineLookup.begin(); it != m_pipelineLookup.end();)
		{
			if (it->second == removedIndex)
			{
				it = m_pipelineLookup.erase(it);
				continue;
			}

			if (it->second > removedIndex)
				--it->second;

			++it;
		}

		// Clear the type look up also
		for (auto it = m_typeToPipelineLookup.begin(); it != m_typeToPipelineLookup.end();)
		{
			if (it->second == removedIndex)
			{
				it = m_typeToPipelineLookup.erase(it);
				continue;
			}

			if (it->second > removedIndex)
				--it->second;

			++it;
		}

		Terminal::Info(StringOps::GetName(this), "{} has been unregistered", pType->GetName());
		return true;
	}

	b8 ImporterRegistry::AddImporterType(const Reflect::Type* pType)
	{
		if (pType->GetBaseId() != Reflect::TypeOf<AssetImporter>())
			return false;

		auto* pAttr = pType->GetCustomAttribute<AssetImporterTypeAttribute>();
		if (!pAttr)
		{
			Terminal::Error(StringOps::GetName(this), "{} needs AssetImporterTypeAttribute to be added. Skipping it..", pType->GetName());
			return false;
		}

		auto* pObject = (AssetImporter*)pType->Create();
		if (!pObject)
		{
			Terminal::Error(StringOps::GetName(this), "{} has virtual function issues. You won't have this importer.", pType->GetName());
			return false;
		}

		ImporterEntry entry = {};
		entry.pImporter = pObject;
		entry.pType = pType;

		m_typeToImporterLookup[pAttr->GetAssetType()] = m_importers.GetCount();
		m_importers.PushBack(entry);

		Terminal::Info(StringOps::GetName(this), "{} has been registered", pType->GetName());
		return true;
	}

	b8 ImporterRegistry::RemoveImporterType(const Reflect::Type* pType)
	{
		if (pType->GetBaseId() != Reflect::TypeOf<AssetImporter>())
			return false;

		usize removedIndex = m_importers.GetCount();
		for (usize i = 0; i < m_importers.GetCount(); ++i)
		{
			if (m_importers[i].pType == pType)
			{
				removedIndex = i;
				break;
			}
		}

		if (removedIndex == m_importers.GetCount())
		{
			Terminal::Warn(StringOps::GetName(this), "{} is not registered to ImporterRegistry", pType->GetName());
			return false;
		}

		Memory::Allocator::Delete(m_importers[removedIndex].pImporter);
		m_importers.RemoveAt(removedIndex);

		for (auto it = m_typeToImporterLookup.begin(); it != m_typeToImporterLookup.end();)
		{
			if (it->second == removedIndex)
			{
				it = m_typeToImporterLookup.erase(it);
				continue;
			}

			if (it->second > removedIndex)
				--it->second;

			++it;
		}

		Terminal::Info(StringOps::GetName(this), "{} has been unregistered", pType->GetName());
		return true;
	}
}