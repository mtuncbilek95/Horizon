#include "AssetService.h"

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/PAL/File/File.h>
#include <Runtime/Containers/StringOps.h>

#include <cstring>

namespace Horizon::Engine
{
	ModuleReport AssetService::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();

		List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<AssetStreamer>());

		for (auto* pType : types)
		{
			AssetStreamer* pStreamer = (AssetStreamer*)pType->Create();
			if (!pStreamer)
			{
				Terminal::Warn(StringOps::GetName(this), "Forgetting to implement a virtual function can cause the previous error!");
				continue;
			}

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

		for (auto* pAsset : m_usableAssets)
			Memory::Allocator::Delete(pAsset);
	}

	void AssetService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
	}

	AssetStreamer* AssetService::FindStreamer(Reflect::TypeHandle handle)
	{
		auto it = m_streamerLookup.find(handle);

		if (it == m_streamerLookup.end())
			return nullptr;

		return m_streamers[it->second];
	}

	b8 AssetService::RegisterAsset(const AssetPhysicalEntry& entry)
	{
		if (m_idLookup.contains(entry.assetId))
			return true;

		auto* pReflect = GetEngine()->GetReflectionSystem();
		auto* pType = pReflect->GetType(entry.assetType);

		AssetObject* pNewAssetObject = (AssetObject*)pType->Create();
		pNewAssetObject->m_streamer = FindStreamer(entry.assetType);
		pNewAssetObject->m_ownerEntry = entry;

		ReadHeader(pNewAssetObject);

		m_idLookup[entry.assetId] = m_usableAssets.GetCount();
		m_usableAssets.PushBack(pNewAssetObject);

		return true;
	}

	b8 AssetService::UnregisterAsset(const Guid& id)
	{
		auto it = m_idLookup.find(id);

		if (it == m_idLookup.end())
			return true;

		Memory::Allocator::Delete(m_usableAssets[it->second]);

		m_usableAssets.RemoveAt(it->second);
		m_idLookup.erase(id);

		return true;
	}

	b8 AssetService::RefreshAsset(const Guid& id)
	{
		auto it = m_idLookup.find(id);

		if (it == m_idLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not registered, nothing to refresh", id.ToString());
			return false;
		}

		return ReadHeader(m_usableAssets[it->second]);
	}

	AssetObject* AssetService::FindAsset(const Guid& id)
	{
		auto it = m_idLookup.find(id);

		if (it == m_idLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} has not been found in any of the sources.", id.ToString());
			return nullptr;
		}

		return m_usableAssets[it->second];
	}

	b8 AssetService::ReadHeader(AssetObject* pAsset)
	{
		List<u8> payload;

		PAL::FileAccessRequest request = PAL::File::RequestAccess(pAsset->m_ownerEntry.cookPath, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		const b8 wasRead = PAL::File::ReadMemory(request, payload, 0, sizeof(AssetHeader));

		PAL::File::ReleaseAccess(request);

		if (!wasRead || payload.GetCount() < sizeof(AssetHeader))
		{
			Terminal::Error(StringOps::GetName(this), "{} has no readable header", pAsset->m_ownerEntry.cookPath);
			return false;
		}

		std::memcpy(&pAsset->m_header, payload.GetData(), sizeof(AssetHeader));

		return true;
	}
}