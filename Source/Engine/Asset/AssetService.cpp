#include "AssetService.h"

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>

namespace Horizon::Engine
{
	ModuleReport AssetService::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();

		List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<AssetStreamer>());

		for (auto* pType : types)
		{
			AssetStreamer* pStreamer = (AssetStreamer*)pType->Create();
			pStreamer->m_engine = GetEngine();
			pStreamer->OnInitialize();

			m_streamerLookup[pStreamer->GetAssetType()] = m_streamers.GetCount();
			m_streamers.PushBack(pStreamer);

			auto* pAssetType = pReflect->GetType(pStreamer->GetAssetType());
			Terminal::Info(StringOps::GetName(this), "{} has been registered for the {} type", pType->GetName(), pAssetType->GetName());
		}

		return ModuleReport();
	}

	void AssetService::OnExecute(const EngineFrame& ctx)
	{
	}

	void AssetService::OnFinalize()
	{
		for (auto* pStreamer : m_streamers)
		{
			pStreamer->OnFinalize();
			Memory::Allocator::Delete(pStreamer);
		}
	}

	void AssetService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
	}

	void AssetService::AddSource(AssetSourceFile* pSource)
	{
		if (pSource == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "A null asset source cannot be added");
			return;
		}

		for (auto* pExisting : m_sources)
		{
			if (pExisting == pSource)
			{
				Terminal::Warn(StringOps::GetName(this), "{} is already added as an asset source", pSource->GetName());
				return;
			}
		}

		m_sources.PushBack(pSource);
	}
	
	AssetStreamer* AssetService::FindStreamer(Reflect::TypeHandle handle)
	{
		auto it = m_streamerLookup.find(handle);
		if (it == m_streamerLookup.end())
			return nullptr;

		return m_streamers[it->second];
	}
}