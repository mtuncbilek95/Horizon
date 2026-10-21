#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Asset/AssetLoadSystem.h>
#include <Engine/Asset/Shader/ShaderAsset.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API ShaderAssetLoader : public AssetLoadSystem
	{
		HORIZON_TYPE_REFLECT(ShaderAssetLoader);
	public:
		ShaderAssetLoader() = default;
		~ShaderAssetLoader() = default;

		Reflect::TypeHandle GetWorkingAssetType() const final { return Reflect::TypeOf<ShaderAsset>(); }

		void LoadAsync(ShaderAsset* pAsset);
		void UnloadAsync(ShaderAsset* pAsset);

		void LoadSync(ShaderAsset* pAsset);
		void UnloadSync(ShaderAsset* pAsset);

	protected:
		void OnInitialize() final;

		void OnRegisteredAsset(const AssetEntry& entry) final;
		void OnUnregisteredAsset(const AssetEntry& entry) final;
		void OnRefreshAsset(const AssetEntry& entry) final;

	private:
		void ReleaseAsset(ShaderAsset* pAsset);

	private:
		GraphicsContext* m_context = nullptr;

		JobSystem* m_jobSystem = nullptr;
		PAL::CriticalSection m_shaderLocker;
	};
}