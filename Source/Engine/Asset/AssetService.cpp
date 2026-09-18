#include "AssetService.h"

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Asset/AssetHeader.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

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

			m_streamerLookup[pType->GetTypeId()] = m_streamers.GetCount();
			m_streamers.PushBack(pStreamer);

			auto* pAssetType = pReflect->GetType(pStreamer->GetAssetType());
			Terminal::Info(StringOps::GetName(this), "{} has been registered for the {} type", pType->GetName(), pAssetType->GetName());
		}

		// If you see this and judge me, FUCK YOU! IT WILL BE AUTOMATIC PLEASE FUCK OFF!
		std::string cookedPath = "D:/Projects/Horizon/ExampleProject/Cooked";

		// TODO: I will most probably move this behaviour somewhere else but lets keep it here for now.
		List<PAL::Directory::Entry> files = PAL::Directory::Iterate(cookedPath);
		for (const auto& file : files)
		{
			if (file.isDirectory)
				continue;

			List<u8> headerBytes;
			PAL::FileAccessRequest handle = PAL::File::RequestAccess(file.fullPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::Exclusive);
			PAL::File::ReadMemory(handle, headerBytes, 0, sizeof(AssetHeader));
			PAL::File::ReleaseAccess(handle);

			if (headerBytes.GetCount() < sizeof(AssetHeader))
			{
				Terminal::Error("AssetService", "'{}' is smaller than an asset header", file.fullPath);
				continue;
			}

			AssetHeader header;
			std::memcpy(&header, headerBytes.GetData(), sizeof(AssetHeader));

			Reflect::Type* pType = pReflect->GetTypeByName(header.typeName);

			AssetEntry entry = {};
			entry.assetId = header.id;
			entry.cookedPath = file.fullPath;
			entry.assetTypeHandle = pType->GetTypeId();
			m_assetEntries[header.id] = entry;

			Terminal::Debug(StringOps::GetName(this), "{} has been registered as usable asset", file.name);
		}

		return ModuleReport();
	}

	void AssetService::OnExecute(const EngineFrame& ctx)
	{
	}

	void AssetService::OnFinalize()
	{
		for (auto* pStreamer : m_streamers)
			Memory::Allocator::Delete(pStreamer);
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