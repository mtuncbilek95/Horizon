#pragma once

#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetLoadSystem : public Reflect::Base
	{
		friend class AssetService;
	public:
		Engine* GetEngine() const { return m_engine; }

		virtual Reflect::TypeHandle GetWorkingAssetType() const = 0;

	protected:
		virtual void OnInitialize() {}
		virtual void OnExecute(const EngineFrame& frame) {}
		virtual void OnFinalize() {}

		virtual void OnRegisteredAsset(const AssetEntry& entry) = 0;
		virtual void OnUnregisteredAsset(const AssetEntry& entry) = 0;
		virtual void OnRefreshAsset(const AssetEntry& entry) = 0;

	private:
		Engine* m_engine = nullptr;
	};
}