#include "AssetModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Job/JobModule.h>

namespace Horizon
{
	void AssetModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		m_registry.LoadFromFile("Assets/List.hregistry");
	}

	void AssetModule::OnSync()
	{
		m_loadQueue.DrainCompleted(m_completedScratch);
		for (const AsyncQueueCompletion& comp : m_completedScratch)
		{
			ResidentSlot& slt = m_slots[comp.slot];
			if (slt.generation != comp.generation)
				continue;

			Publish(comp.slot, comp.asset);
		}

		m_freeScratch.clear();
		for (const u32 slot : m_pendingFree)
		{
			ResidentSlot& s = m_slots[slot];

			if (s.refCount != 0) 
				continue;

			if (s.state.load(std::memory_order_acquire) == LoadState::Pending)
			{
				m_freeScratch.push_back(slot); 
				continue;
			}

			if (s.asset)
				m_typeRegistry.At(s.typeIndex).loader->Unload(s.asset);

			s.asset = nullptr;
			m_index.erase(s.guid);
			++s.generation;
			m_freeList.push_back(slot);
		}

		m_pendingFree.swap(m_freeScratch);
	}

	void AssetModule::OnDetach()
	{
		m_registry.SaveToFile("Assets/List.hregistry");
	}

	b8 AssetModule::IsResident(Guid guid) const
	{
		auto it = m_index.find(guid);
		return it != m_index.end()
			&& m_slots[it->second].state.load(std::memory_order_acquire) == LoadState::Ready;
	}

	u16 AssetModule::RegisterType(const AssetTypeDesc& desc)
	{
		return m_typeRegistry.Register(desc);
	}

	void AssetModule::IncrementRef(u32 slot)
	{
		++m_slots[slot].refCount;
	}

	void AssetModule::DecrementRef(u32 slot)
	{
		if (--m_slots[slot].refCount == 0)
			m_pendingFree.push_back(slot);
	}

	b8 AssetModule::IsSlotReady(u32 slot, u32 gen) const
	{
		const ResidentSlot& slt = m_slots[slot];
		return slt.generation == gen
			&& slt.state.load(std::memory_order_acquire) == LoadState::Ready;
	}

	const void* AssetModule::ResolveReady(u32 slot, u32 gen) const
	{
		const ResidentSlot& slt = m_slots[slot];
		if (slt.generation != gen)
			return nullptr;

		if (slt.state.load(std::memory_order_acquire) != LoadState::Ready)
			return DefaultAssetFor(slt.typeIndex);

		return slt.asset;
	}

	Guid AssetModule::GuidOf(u32 slot, u32 gen) const
	{
		const ResidentSlot& slt = m_slots[slot];
		return slt.generation == gen ? slt.guid : Guid{};
	}

	u32 AssetModule::AcquireSlot(Guid guid, u16 typeIndex)
	{
		u32 slot;
		if (!m_freeList.empty())
		{
			slot = m_freeList.back();
			m_freeList.pop_back();
		}
		else
		{
			slot = static_cast<u32>(m_slots.size());
			m_slots.emplace_back();
		}

		ResidentSlot& slt = m_slots[slot];
		slt.guid = guid;
		slt.typeIndex = typeIndex;
		slt.refCount = 0;
		slt.asset = nullptr;
		slt.state.store(LoadState::Pending, std::memory_order_relaxed);

		m_index[guid] = slot;
		return slot;
	}

	void AssetModule::QueueLoad(u32 slot, b8 async)
	{
		if (!async)
		{
			Publish(slot, LoadInto(slot));
			return;
		}

		const u32 gen = m_slots[slot].generation;
		auto& jobSys = m_engine->GetModule<JobModule>();
		jobSys.SubmitJob([this, slot, gen]
			{
				void* asset = LoadInto(slot);
				m_loadQueue.PushCompleted({ slot, gen, asset });
			});
	}

	void* AssetModule::LoadInto(u32 slot)
	{
		const Guid guid = m_slots[slot].guid;
		const std::optional<AssetMetadata> meta = m_registry.Find(guid);
		if (!meta) 
			return nullptr;

		const AssetTypeDesc& desc = m_typeRegistry.At(m_slots[slot].typeIndex);
		if (meta->type != desc.id || !desc.loader) 
			return nullptr;

		AssetLoadContext ctx = {};
		ctx.pModule = this;
		ctx.guid = guid;
		ctx.cookedPath = meta->cookedPath;
		ctx.dependencies = &meta->dependencies;

		return desc.loader->Load(ctx);
	}

	void AssetModule::Publish(u32 slot, void* asset)
	{
		m_slots[slot].asset = asset;
		m_slots[slot].state.store(asset ? LoadState::Ready : LoadState::Failed,
			std::memory_order_release);
	}

	const void* AssetModule::DefaultAssetFor(u32 typeId) const
	{
		return nullptr;
	}
}