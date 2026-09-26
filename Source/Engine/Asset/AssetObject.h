#pragma once

#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/AssetPhysicalEntry.h>
#include <Engine/Asset/AssetHeader.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetStreamer;

	class H_EXPORT AssetObject : public Reflect::Base
	{
		friend class AssetService;
	public:
		virtual ~AssetObject() = default;

		AssetResidency GetResidencyState() const { return m_residency.Load(); }

		template<typename T>
		T* GetStreamer() const { return (T*)GetStreamer(); }
		AssetStreamer* GetStreamer() const { return m_streamer; }

		const AssetPhysicalEntry& GetPhysicalEntry() const { return m_ownerEntry; }

	protected:
		AssetPhysicalEntry m_ownerEntry;
		AssetHeader m_header;

		AssetStreamer* m_streamer = nullptr;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;
	};
}