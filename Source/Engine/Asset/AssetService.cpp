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
		for (auto& [id, pObject] : m_objects)
		{
			if (pObject)
				pObject->GetStreamer()->Unload(pObject);
		}

		m_objects.clear();

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

	AssetObject* AssetService::RequestObject(const Guid& id, Reflect::TypeHandle assetType)
	{
		auto it = m_objects.find(id);

		if (it != m_objects.end())
			return it->second;

		const AssetEntry* pEntry = FindEntry(id);

		if (pEntry == nullptr)
			return nullptr;

		AssetObject* pObject = nullptr;

		if (!(pEntry->assetTypeHandle == assetType))
		{
			Terminal::Error(StringOps::GetName(this), "{} was requested with a type that does not match its entry", id.ToString());
		}
		else
		{
			AssetStreamer* pStreamer = FindStreamer(assetType);

			if (pStreamer == nullptr)
				Terminal::Error(StringOps::GetName(this), "{} has no streamer for its asset type", id.ToString());
			else
				pObject = pStreamer->Load(*pEntry);
		}

		m_objects[id] = pObject;

		return pObject;
	}

	const AssetEntry* AssetService::FindEntry(const Guid& id) const
	{
		for (auto* pSource : m_sources)
		{
			const AssetEntry* pEntry = pSource->Find(id);

			if (pEntry)
				return pEntry;
		}

		return nullptr;
	}

	AssetStreamer* AssetService::FindStreamer(Reflect::TypeHandle handle)
	{
		auto it = m_streamerLookup.find(handle);
		if (it == m_streamerLookup.end())
			return nullptr;

		return m_streamers[it->second];
	}
}