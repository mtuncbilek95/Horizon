#include "AssetService.h"

#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/PAL/File/File.h>
#include <Runtime/Containers/StringOps.h>

namespace Horizon::Engine
{
	ModuleReport AssetService::OnInitialize()
	{
		m_reflection = GetEngine()->GetReflectionSystem();

		for (const Reflect::Type* pType : m_reflection->GetTypeByBase(Reflect::TypeOf<AssetLoadSystem>()))
			AddLoaderType(pType);

		return ModuleReport();
	}

	void AssetService::OnExecute(const EngineFrame& ctx)
	{
		for (StreamerEntry& entry : m_streamers)
			entry.pLoader->OnExecute(ctx);
	}

	void AssetService::OnFinalize()
	{
		for (AssetEntry& entry : m_usableAssets)
		{
			if (entry.pAssetObject->m_loader)
				entry.pAssetObject->m_loader->OnUnregisteredAsset(entry);

			Memory::Allocator::Delete(entry.pAssetObject);
		}

		m_usableAssets.Clear();
		m_idLookup.clear();
		m_relativePathLookup.clear();

		for (StreamerEntry& entry : m_streamers)
		{
			entry.pLoader->OnFinalize();
			Memory::Allocator::Delete(entry.pLoader);
		}

		m_streamers.Clear();
		m_streamerLookup.clear();
	}

	void AssetService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
	}

	void AssetService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
		{
			if (type.GetBaseId() == Reflect::TypeOf<AssetLoadSystem>())
				AddLoaderType(&type);
		}
	}

	void AssetService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			RemoveLoaderType(&type);
	}

	b8 AssetService::AddLoaderType(const Reflect::Type* pType)
	{
		AssetLoadSystem* pLoader = (AssetLoadSystem*)pType->Create();

		if (!pLoader)
		{
			Terminal::Warn(StringOps::GetName(this), "Forgetting to implement a virtual function can cause the previous error!");
			return false;
		}

		pLoader->m_engine = GetEngine();
		pLoader->OnInitialize();

		m_streamerLookup[pLoader->GetWorkingAssetType()] = pLoader;
		m_streamers.EmplaceBack(pType, pLoader);

		auto* pAssetType = GetEngine()->GetReflectionSystem()->GetType(pLoader->GetWorkingAssetType());
		Terminal::Info(StringOps::GetName(this), "{} has been registered for the {} type", pType->GetName(), pAssetType ? pAssetType->GetName() : "unknown");
		return true;
	}

	void AssetService::RemoveLoaderType(const Reflect::Type* pType)
	{
		for (usize i = m_streamers.GetCount(); i > 0; --i)
		{
			StreamerEntry& entry = m_streamers[i - 1];

			if (entry.pType != pType)
				continue;

			m_streamerLookup.erase(entry.pLoader->GetWorkingAssetType());
			entry.pLoader->OnFinalize();
			Memory::Allocator::Delete(entry.pLoader);
			m_streamers.RemoveAt(i - 1);
		}
	}

	b8 AssetService::RegisterAsset(const Guid& id, const std::string& cookPath, const std::string& callPath, const std::string& name, Reflect::TypeHandle type)
	{
		auto itAsset = m_idLookup.find(id);
		if (itAsset != m_idLookup.end())
		{
			Terminal::Warn(StringOps::GetName(this), "{} | {} has been already registered", id.ToString(), cookPath);
			return true;
		}

		Reflect::Type* pType = m_reflection->GetType(type);

		auto itStreamer = m_streamerLookup.find(type);
		if (itStreamer == m_streamerLookup.end())
		{
			Terminal::Warn(StringOps::GetName(this), "{} has any type of loader", pType->GetName());
			return false;
		}

		AssetEntry entry = {};
		entry.assetId = id;
		entry.assetType = type;
		entry.assetTypeName = pType->GetName();
		entry.cookPath = cookPath;
		entry.callPath = callPath;
		entry.pAssetObject = (AssetObject*)pType->Create();

		if (!entry.pAssetObject)
		{
			Terminal::Error(StringOps::GetName(this), "{} | {} has virtual function issues. Please check it out", id.ToString(), pType->GetName());
			return false;
		}
		entry.pAssetObject->m_loader = itStreamer->second;

		m_relativePathLookup[callPath] = m_usableAssets.GetCount();
		m_idLookup[id] = m_usableAssets.GetCount();
		m_usableAssets.PushBack(entry);

		itStreamer->second->OnRegisteredAsset(m_usableAssets.Back());
		return true;
	}

	b8 AssetService::UnregisterAsset(const Guid& id)
	{
		auto itAsset = m_idLookup.find(id);
		if (itAsset == m_idLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not registered, nothing to unregister", id.ToString());
			return true;
		}

		const usize index = itAsset->second;
		AssetEntry& entry = m_usableAssets[index];

		if (entry.pAssetObject->m_loader)
			entry.pAssetObject->m_loader->OnUnregisteredAsset(entry);

		m_relativePathLookup.erase(entry.callPath);
		m_idLookup.erase(itAsset);
		Memory::Allocator::Delete(entry.pAssetObject);

		const usize lastIndex = m_usableAssets.GetCount() - 1;
		if (index != lastIndex)
		{
			m_usableAssets[index] = std::move(m_usableAssets[lastIndex]);
			m_idLookup[m_usableAssets[index].assetId] = index;
			m_relativePathLookup[m_usableAssets[index].callPath] = index;
		}

		m_usableAssets.PopBack();
		return true;
	}

	b8 AssetService::RefreshAsset(const Guid& id)
	{
		auto itAsset = m_idLookup.find(id);
		if (itAsset == m_idLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not registered, nothing to refresh", id.ToString());
			return false;
		}

		AssetEntry& entry = m_usableAssets[itAsset->second];

		if (entry.pAssetObject->m_loader)
			entry.pAssetObject->m_loader->OnRefreshAsset(entry);

		return true;
	}

	AssetObject* AssetService::FindAsset(const Guid& id) const
	{
		auto itAsset = m_idLookup.find(id);
		if (itAsset == m_idLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} has not been found in any of the sources", id.ToString());
			return nullptr;
		}

		return m_usableAssets[itAsset->second].pAssetObject;
	}

	AssetObject* AssetService::FindAsset(const Guid& id, Reflect::TypeHandle type) const
	{
		auto itAsset = m_idLookup.find(id);
		if (itAsset == m_idLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} has not been found in any of the sources", id.ToString());
			return nullptr;
		}

		const AssetEntry& entry = m_usableAssets[itAsset->second];
		if (entry.assetType != type)
		{
			Terminal::Error(StringOps::GetName(this), "{} is a {}, not the requested asset type", id.ToString(), entry.assetTypeName);
			return nullptr;
		}

		return entry.pAssetObject;
	}

	AssetObject* AssetService::FindAssetByPath(const std::string& callPath) const
	{
		auto itAsset = m_relativePathLookup.find(callPath);
		if (itAsset == m_relativePathLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} has not been found in any of the sources", callPath);
			return nullptr;
		}

		return m_usableAssets[itAsset->second].pAssetObject;
	}

	AssetLoadSystem* AssetService::FindLoader(Reflect::TypeHandle assetType) const
	{
		auto itLoader = m_streamerLookup.find(assetType);
		if (itLoader == m_streamerLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "No loader is registered for the requested asset type");
			return nullptr;
		}

		return itLoader->second;
	}
}