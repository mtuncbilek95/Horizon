#pragma once

#include <Engine/Asset/AssetResidency.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetStreamer;

	class H_EXPORT AssetObject : public Reflect::Base
	{
		friend class AssetStreamer;
	public:
		virtual ~AssetObject() = default;

		const Guid& GetId() const { return m_id; }
		u32 GetSlot() const { return m_slot; }

		AssetResidency GetState() const { return m_state.Load(); }

		template<typename T>
		T* GetStreamer() const { return (T*)GetStreamer(); }
		AssetStreamer* GetStreamer() const { return m_streamer; }

	private:
		Guid m_id;
		AssetStreamer* m_streamer = nullptr;

		u32 m_slot = kInvalid32;
		PAL::Atomic<AssetResidency> m_state = AssetResidency::Unloaded;
		PAL::Atomic<u32> m_refCount = 0;
		PAL::Atomic<u32> m_useCount = 0;
	};
}