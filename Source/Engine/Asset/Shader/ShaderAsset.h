#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/Shader/ShaderAssetProperties.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetKind::Data]);
	class ENGINE_API ShaderAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(ShaderAsset);
		friend class ShaderAssetLoader;
	public:
		ShaderAsset() = default;
		~ShaderAsset() = default;

		void LoadSync();
		void UnloadSync();

		const Guid& GetId() const { return m_id; }
		AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

		const MaterialRenderState& GetRenderState() const { return m_properties.renderState; }
		RHI::GfxPipeline* GetPipeline() const { return m_pipeline; }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;

		ShaderAssetProperties m_properties;
		RHI::GfxPipeline* m_pipeline = nullptr;
	};
}