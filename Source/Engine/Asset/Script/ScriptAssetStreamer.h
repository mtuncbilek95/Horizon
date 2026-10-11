#pragma once

#include <Engine/Asset/AssetStreamer.h>
#include <Engine/Asset/Script/ScriptAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class ReflectionSystem;

	HCLASS();
	class ENGINE_API ScriptAssetStreamer : public AssetStreamer
	{
		HORIZON_TYPE_REFLECT(ScriptAssetStreamer);
	public:
		ScriptAssetStreamer() = default;
		~ScriptAssetStreamer() = default;

		void OnInitialize() final;
		void OnSync(const EngineFrame& frameContext) final;
		void OnFinalize() final;

		void LoadAsync(AssetObject* pAsset) final;
		void UnloadAsync(AssetObject* pAsset) final;

		Reflect::TypeHandle GetAssetType() final { return Reflect::TypeOf<ScriptAsset>(); }

	private:
		void RunLoadAsset(ScriptAsset* pAsset);

	private:
		JobSystem* m_jobSystem = nullptr;
		ReflectionSystem* m_reflection = nullptr;
	};
}