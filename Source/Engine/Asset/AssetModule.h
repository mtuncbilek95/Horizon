#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/AsyncLoadQueue.h>
#include <Engine/Asset/AssetTypeRegistry.h>
#include <Engine/Asset/AssetRegistry.h>
#include <Engine/Asset/AssetLoader.h>

#include <deque>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace Horizon
{
	class AssetModule final : public IModule
	{
		template<typename T>
		friend class AssetHandle;

		enum class LoadState : u8
		{
			Pending,
			Ready,
			Failed
		};

		struct ResidentSlot
		{
			Guid guid;
			u32 generation = 0;
			u32 refCount = 0;
			u16 typeIndex = kInvalid16;
			std::atomic<LoadState> state = LoadState::Pending;
			void* asset = nullptr;
		};

	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

		u16 RegisterType(const AssetTypeDesc& desc);

		template<typename T>
		AssetHandle<T> Load(Guid guid) { return AssetHandle<T>(); }

		template<typename T>
		AssetHandle<T> LoadAsync(Guid guid) { return AssetHandle<T>(); }

		b8 IsResident(Guid guid) const;

	private:
		void IncrementRef(u32 slot);
		void DecrementRef(u32 slot);

		b8 IsSlotReady(u32 slot, u32 gen) const;

		const void* ResolveReady(u32 slot, u32 gen) const;

		Guid GuidOf(u32 slot, u32 gen) const;

		template<typename T>
		AssetHandle<T> MakeHandle(Guid guid, b8 async)
		{
			if (!guid.IsValid())
				return AssetHandle<T>();

			const u16 typeIndex = m_typeRegistry.IndexOf(AssetTypeOf<T>());
			assert(typeIndex != kInvalid16 && "Type is not registered");

			u32 slot;
			u32 gen;
			b8  needsLoad = false;

			{
				std::lock_guard lock(m_tableMutex);

				auto it = m_index.find(guid);

				if (it != m_index.end())
					slot = it->second;
				else
				{
					slot = AcquireSlot(guid, typeIndex);
					needsLoad = true;
				}
				gen = m_slots[slot].generation;
			}

			if (needsLoad)
				QueueLoad(slot, async);

			return AssetHandle<T>(this, slot, gen);
		}

		u32 AcquireSlot(Guid guid, u16 typeIndex);
		void QueueLoad(u32 slot, b8 async);
		void* LoadInto(u32 slot);
		void Publish(u32 slot, void* asset);
		const void* DefaultAssetFor(u32 typeId) const;

	private:
		AssetRegistry m_registry;
		AssetTypeRegistry m_typeRegistry;
		// TODO: LoaderRegistry m_loaders;

		std::deque<ResidentSlot> m_slots;
		std::unordered_map<Guid, u32, GuidHash> m_index;
		std::vector<u32> m_freeList;
		std::vector<u32> m_pendingFree;

		std::mutex m_tableMutex;

		AsyncLoadQueue m_loadQueue;
		std::vector<AsyncQueueCompletion> m_completedScratch;
		std::vector<u32> m_freeScratch;
	};
}