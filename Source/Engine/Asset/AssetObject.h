#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon::Engine
{
	class Application;
	struct AssetTypeDesc;

	class H_EXPORT AssetObject
	{
		friend class AssetSystem;
	public:
		AssetObject() = default;
		virtual ~AssetObject() = default;

		AssetObject(const AssetObject&) = delete;
		AssetObject& operator=(const AssetObject&) = delete;

		const Guid& GetID() const { return m_guid; }

		Application* GetEngine() const { return m_engine; }
		const AssetTypeDesc* GetTypeDesc() const { return m_typeDesc; }
		const std::string& GetDebugName() const { return m_debugName; }

		u32 GetRefCount() const { return m_refCount.Load(); }

	private:
		Application* m_engine = nullptr;
		const AssetTypeDesc* m_typeDesc = nullptr;

		Guid m_guid;
		std::string m_debugName;

		PAL::Atomic<u32> m_refCount = 0;
	};
}