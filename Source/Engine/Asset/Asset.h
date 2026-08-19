#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	class H_EXPORT Asset
	{
		friend class AssetService;
	public:
		Asset() = default;
		virtual ~Asset() = default;

		const Guid& GetID() const { return m_guid; }
		u32 GetRefCount() const { return m_refCount; }

	private:
		Guid m_guid;
		u32 m_refCount = 0;
	};
}