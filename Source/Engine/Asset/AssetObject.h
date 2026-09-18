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

		AssetResidency GetState() const { return m_state; }

		template<typename T>
		T* GetStreamer() const { return (T*)GetStreamer(); }
		AssetStreamer* GetStreamer() const { return m_streamer; }

	private:
		Guid m_id;
		AssetStreamer* m_streamer = nullptr;

		AssetResidency m_state = AssetResidency::Unloaded;
	};
}