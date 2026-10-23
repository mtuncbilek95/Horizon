#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Asset/AssetEntry.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class ENGINE_API AssetService : public Service
	{
		struct StreamerEntry
		{
			const Reflect::Type* pType = nullptr;
			AssetLoadSystem* pLoader = nullptr;
		};
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

		b8 RegisterAsset(const Guid& id, const std::string& cookPath, const std::string& callPath, const std::string& name, Reflect::TypeHandle type);
		b8 UnregisterAsset(const Guid& id);
		b8 RefreshAsset(const Guid& id);
		b8 HasAsset(const Guid& id) const { return m_idLookup.contains(id); }

		AssetObject* FindAsset(const Guid& id) const;
		AssetObject* FindAsset(const Guid& id, Reflect::TypeHandle type) const;
		AssetObject* FindAssetByPath(const std::string& callPath) const;
		AssetLoadSystem* FindLoader(Reflect::TypeHandle assetType) const;

		template<typename T>
		T* FindAsset(const Guid& id) const { return (T*)FindAsset(id, Reflect::TypeOf<T>()); }

		template<typename T>
		T* FindLoader(Reflect::TypeHandle assetType) const { return (T*)FindLoader(assetType); }

	private:
		b8 AddLoaderType(const Reflect::Type* pType);
		void RemoveLoaderType(const Reflect::Type* pType);

	private:
		ReflectionSystem* m_reflection = nullptr;

		List<StreamerEntry> m_streamers;
		std::unordered_map<Reflect::TypeHandle, AssetLoadSystem*> m_streamerLookup;

		List<AssetEntry> m_usableAssets;
		std::unordered_map<Guid, usize> m_idLookup;
		std::unordered_map<std::string, usize> m_relativePathLookup;
	};
}