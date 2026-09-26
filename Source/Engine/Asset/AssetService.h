#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Asset/AssetPhysicalEntry.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/AssetObject.h>
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

		b8 RegisterAsset(const AssetPhysicalEntry& entry);
		b8 UnregisterAsset(const Guid& id);

		template<typename T>
		T* FindAsset(const Guid& id)
		{
			return (T*)FindAsset(id);
		}
		AssetObject* FindAsset(const Guid& id);
		AssetStreamer* FindStreamer(Reflect::TypeHandle handle);

	private:
		List<AssetStreamer*> m_streamers;
		std::unordered_map<Reflect::TypeHandle, usize> m_streamerLookup;

		List<AssetObject*> m_usableAssets;
		std::unordered_map<Guid, usize> m_idLookup;
	};
}