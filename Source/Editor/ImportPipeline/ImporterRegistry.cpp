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

		m_pipelines.Clear();
		m_extToPipelineLookup.clear();
		m_typeToPipelineLookup.clear();

		m_importers.Clear();
		m_typeToImporterLookup.clear();
		m_extToImporterLookup.clear();
	}

	void ImporterRegistry::BootstrapViews(Engine::Engine* ctx)
	{
		m_engine = ctx;

		m_reflection = ctx->GetReflectionSystem();

		// Register the Import Pipeline processor mechs
		for (auto* pType : m_reflection->GetTypeByBase(Reflect::TypeOf<ImportPipeline>()))
			AddPipelineType(pType);

		// Register the importers such as ufbx/fast-gltf/ktx etc
		for (auto* pType : m_reflection->GetTypeByBase(Reflect::TypeOf<AssetImporter>()))
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

	ImportPipelineEntry* ImporterRegistry::GetImportPipelineEntry(const std::string& extension)
	{
		auto it = m_extToPipelineLookup.find(extension);
		if (it == m_extToPipelineLookup.end())
			return nullptr;

		return &m_pipelines[it->second];
	}

	ImportPipelineEntry* ImporterRegistry::GetImportPipelineEntry(Reflect::TypeHandle handle)
	{
		auto it = m_typeToPipelineLookup.find(handle);
		if (it == m_typeToPipelineLookup.end())
			return nullptr;

		return &m_pipelines[it->second];
	}

	AssetImporterEntry* ImporterRegistry::GetAssetImporterEntry(Reflect::TypeHandle handle)
	{
		auto it = m_typeToImporterLookup.find(handle);
		if (it == m_typeToImporterLookup.end())
			return nullptr;

		return &m_importers[it->second];
	}

	AssetImporter* ImporterRegistry::CreateImporterInstance(const std::string& ext)
	{
		auto it = m_extToImporterLookup.find(ext);
		if (it == m_extToImporterLookup.end())
			return nullptr;

		auto* pType = m_importers[it->second].pImportType;

		// Create one instance for each call to use importers freely.
		auto* pAssetImporter = (AssetImporter*)pType->Create();
		if (!pAssetImporter)
		{
			Terminal::Warn(StringOps::GetName(this), "This could be about missing virtual functions, check interface's virtuals then check if your importer has HORIZON_TYPE_REFLECT");
			return nullptr;
		}

		return pAssetImporter;
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

		ImportPipelineEntry entry = {};
		entry.pPipeline = pObject;
		entry.pPipelineType = const_cast<Reflect::Type*>(pType);
		entry.pAssetType = m_reflection->GetType(pAttr->GetWorkingAssetType());
		entry.pSettingsType = m_reflection->GetType(pAttr->GetSettingsType());
		entry.extensions = pAttr->GetExtensions();

		m_typeToPipelineLookup[pAttr->GetWorkingAssetType()] = m_pipelines.GetCount();
		for (const auto& ext : pAttr->GetExtensions())
			m_extToPipelineLookup[ext] = m_pipelines.GetCount();

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
			if (m_pipelines[i].pPipelineType == pType)
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
		for (auto it = m_extToPipelineLookup.begin(); it != m_extToPipelineLookup.end();)
		{
			if (it->second == removedIndex)
			{
				it = m_extToPipelineLookup.erase(it);
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

		AssetImporterEntry entry = {};
		entry.pImportType = const_cast<Reflect::Type*>(pType);
		entry.pAssetType = m_reflection->GetType(pAttr->GetAssetType());
		entry.extensions = pAttr->GetExtensions();
		
		for (const auto& ext : pAttr->GetExtensions())
			m_extToImporterLookup[ext] = m_importers.GetCount();

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
			if (m_importers[i].pImportType == pType)
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

		for (auto it = m_extToImporterLookup.begin(); it != m_extToImporterLookup.end();)
		{
			if (it->second == removedIndex)
			{
				it = m_extToImporterLookup.erase(it);
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