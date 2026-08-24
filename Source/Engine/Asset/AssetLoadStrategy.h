#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Asset;
	class Engine;

	class H_EXPORT AssetLoadStrategy : public Reflect::Base
	{
	public:
		virtual ~AssetLoadStrategy() = default;

		void RegisterStrategy(Engine* pEngine)
		{
			m_engine = pEngine;
			OnRegister(pEngine);
		}

		void UnregisterStrategy(Engine* pEngine)
		{
			m_engine = nullptr;
			OnUnregister(pEngine);
		}

		virtual Asset* Create(const List<u8>& payload) = 0;
		virtual void Destroy(Asset* pAsset) = 0;

		virtual b8 Activate(Asset* pAsset) = 0;
		virtual void Deactivate(Asset* pAsset) = 0;

		virtual Reflect::TypeHandle GetWorkingAssetHandle() = 0;

		Engine* GetEngine() const noexcept { return m_engine; }

	protected:
		virtual void OnRegister(Engine* pEngine) = 0;
		virtual void OnUnregister(Engine* pEngine) = 0;

	private:
		Engine* m_engine = nullptr;
	};
}