#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Asset/AssetObject.h>
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
		virtual void OnFinalize() = 0;

		virtual Reflect::TypeHandle GetAssetType() = 0;

		virtual AssetObject* Load(const AssetEntry& entry) = 0;
		virtual void Unload(AssetObject* pObject) = 0;

	protected:
		void Bind(AssetObject* pObject, const Guid& id, AssetResidency state)
		{
			pObject->m_id = id;
			pObject->m_streamer = this;
			pObject->m_state = state;
		}

	private:
		Engine* m_engine = nullptr;
	};
}