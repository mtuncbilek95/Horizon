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

	class ENGINE_API AssetObject : public Reflect::Base
	{
		friend class AssetService;
		friend class AssetStreamer;
	public:
		virtual ~AssetObject() = default;

		AssetResidency GetResidencyState() const { return m_residency.Load(); }
		u32 GetRevision() const { return m_revision.Load(); }
		b8 IsStale() const { return m_loadedRevision != m_revision.Load(); }

		template<typename T>
		T* GetStreamer() const { return (T*)GetStreamer(); }
		AssetStreamer* GetStreamer() const { return m_streamer; }

		const AssetPhysicalEntry& GetPhysicalEntry() const { return m_ownerEntry; }
	
	protected:
		virtual void OnPropertiesLoaded(ReadOnlyList<u8> propertyBytes) = 0;

	protected:
		AssetPhysicalEntry m_ownerEntry;
		AssetHeader m_header;

		AssetStreamer* m_streamer = nullptr;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;
		PAL::Atomic<u32> m_revision = 0;
		u32 m_loadedRevision = 0;
	};
}