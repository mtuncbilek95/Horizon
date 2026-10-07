#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class ENGINE_API AssetStreamer : public Reflect::Base
	{
		friend class AssetService;
	public:
		Engine* GetEngine() const { return m_engine; }

		virtual void OnInitialize() = 0;
		virtual void OnSync(const EngineFrame& frameContext) = 0;
		virtual void OnFinalize() = 0;

		virtual void LoadAsync(AssetObject* pAsset) = 0;
		virtual void UnloadAsync(AssetObject* pAsset) = 0;

		virtual Reflect::TypeHandle GetAssetType() = 0;

		void FailedAssetLog(AssetObject* pAsset, std::string_view reason);

	private:
		Engine* m_engine = nullptr;
	};
}