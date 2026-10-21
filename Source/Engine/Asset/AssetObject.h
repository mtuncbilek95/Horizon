#pragma once

#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/AssetHeader.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetLoadSystem;

	class ENGINE_API AssetObject : public Reflect::Base
	{
		friend class AssetService;
	public:
		virtual ~AssetObject() = default;

		template<typename T>
		T* GetLoader() const { return (T*)GetLoader(); }
		AssetLoadSystem* GetLoader() const { return m_loader; }

	protected:
		virtual void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) = 0;

	private:
		AssetLoadSystem* m_loader = nullptr;
	};
}