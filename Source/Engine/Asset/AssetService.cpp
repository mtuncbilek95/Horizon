#include "AssetService.h"

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Plugin/PluginService.h>

#include <Runtime/PAL/File/File.h>
#include <Runtime/Containers/StringOps.h>

#include <cstring>

namespace Horizon::Engine
{
	ModuleReport AssetService::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();

		for (const Reflect::Type* pType : pReflect->GetTypeByBase(Reflect::TypeOf<AssetStreamer>()))
			AddStreamerType(pType);

		return ModuleReport();
	}

	void AssetService::OnExecute(const EngineFrame& ctx)
	{
		for (StreamerEntry& entry : m_streamers)
			entry.pStreamer->OnSync(ctx);
	}

	void AssetService::OnFinalize()
	{
		for (StreamerEntry& entry : m_streamers)
		{
			entry.pStreamer->OnFinalize();
			Memory::Allocator::Delete(entry.pStreamer);
		}

		for (auto* pAsset : m_usableAssets)
			Memory::Allocator::Delete(pAsset);
	}

	void AssetService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
		graph.Requires<PluginService>();
	}

	void AssetService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
		{
			if (type.GetBaseId() == Reflect::TypeOf<AssetStreamer>())
				AddStreamerType(&type);
		}
	}

	void AssetService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			RemoveStreamerType(&type);
	}

	AssetStreamer* AssetService::FindStreamer(Reflect::TypeHandle handle)
	{
		auto it = m_streamerLookup.find(handle);

		if (it == m_streamerLookup.end())
			return nullptr;

		return it->second;
	}

	b8 AssetService::AddStreamerType(const Reflect::Type* pType)
	{
		AssetStreamer* pStreamer = (AssetStreamer*)pType->Create();

		if (!pStreamer)
		{
			Terminal::Warn(StringOps::GetName(this), "Forgetting to implement a virtual function can cause the previous error!");
			return false;
		}

		pStreamer->m_engine = GetEngine();
		pStreamer->OnInitialize();

		m_streamerLookup[pStreamer->GetAssetType()] = pStreamer;
		m_streamers.EmplaceBack(pType, pStreamer);

		auto* pAssetType = GetEngine()->GetReflectionSystem()->GetType(pStreamer->GetAssetType());
		Terminal::Info(StringOps::GetName(this), "{} has been registered for the {} type", pType->GetName(), pAssetType ? pAssetType->GetName() : "unknown");
		return true;
	}

	void AssetService::RemoveStreamerType(const Reflect::Type* pType)
	{
		for (usize i = m_streamers.GetCount(); i > 0; --i)
		{
			StreamerEntry& entry = m_streamers[i - 1];

			if (entry.pType != pType)
				continue;

			m_streamerLookup.erase(entry.pStreamer->GetAssetType());
			entry.pStreamer->OnFinalize();
			Memory::Allocator::Delete(entry.pStreamer);
			m_streamers.RemoveAt(i - 1);
		}
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