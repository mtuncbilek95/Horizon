#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Asset/AssetSourceFile.h>
#include <Engine/Asset/AssetEntry.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/AssetStreamer.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class H_EXPORT AssetService : public Service
	{
	public:
		AssetService() = default;
		~AssetService() = default;

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void AddSource(AssetSourceFile* pSource);

		template<typename T>
		AssetHandle<T> RequestAsset(const Guid& id)
		{
			return AssetHandle<T>(id, static_cast<T*>(RequestObject(id, Reflect::TypeOf<T>())));
		}

	private:
		AssetObject* RequestObject(const Guid& id, Reflect::TypeHandle assetType);
		const AssetEntry* FindEntry(const Guid& id) const;
		AssetStreamer* FindStreamer(Reflect::TypeHandle handle);

	private:
		List<AssetStreamer*> m_streamers;
		std::unordered_map<Reflect::TypeHandle, usize> m_streamerLookup;

		std::unordered_map<Guid, AssetObject*> m_objects;

		List<AssetSourceFile*> m_sources;
	};
}