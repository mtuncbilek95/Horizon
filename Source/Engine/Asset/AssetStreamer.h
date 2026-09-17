#pragma once

#include <Engine/Asset/AssetResidency.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT AssetStreamer : public Reflect::Base
	{
		friend class AssetService;
	public:
		Engine* GetEngine() const { return m_engine; }

		virtual void OnInitialize() = 0;
		virtual void OnPublish() = 0;
		virtual void OnFinalize() = 0;

		virtual Reflect::TypeHandle GetAssetType() = 0;

		u32 Request(const Guid& id);
		void Release(const Guid& id);
		AssetResidency GetState(const Guid& id) const;
		u32 GetSlot(const Guid& id) const;

	private:
		Engine* m_engine;
	};
}