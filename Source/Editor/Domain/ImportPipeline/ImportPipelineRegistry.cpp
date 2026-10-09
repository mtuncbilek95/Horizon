#include "ImportPipelineRegistry.h"

#include <Editor/Domain/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/Domain/ImportPipeline/ImportPipeline.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>

namespace Horizon::Editor
{
	ImportPipelineRegistry::~ImportPipelineRegistry()
	{
	}

	void ImportPipelineRegistry::BootstrapViews(Engine::Engine* ctx)
	{
		m_engine = ctx;

		auto* pReflection = ctx->GetReflectionSystem();
		for (auto* pType : pReflection->GetTypeByBase(Reflect::TypeOf<ImportPipeline>()))
			AddType(pType);
	}

	void ImportPipelineRegistry::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			AddType(&type);
	}

	void ImportPipelineRegistry::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			RemoveType(&type);
	}

	b8 ImportPipelineRegistry::AddType(const Reflect::Type* pType)
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
		entry.pType = pType;
		entry.extensions = pAttr->GetExtensions();
		m_pipelines.PushBack(entry);
		Terminal::Info(StringOps::GetName(this), "{} has been registered", pType->GetName());

		return true;
	}

	b8 ImportPipelineRegistry::RemoveType(const Reflect::Type* pType)
	{
		for (usize i = m_pipelines.GetCount(); i > 0; --i)
		{
			ImportPipelineEntry& entry = m_pipelines[i - 1];

			if (entry.pType != pType)
				continue;

			Memory::Allocator::Delete(entry.pPipeline);
			m_pipelines.RemoveAt(i - 1);
			return true;
		}

		return false;
	}
}