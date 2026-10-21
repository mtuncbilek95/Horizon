#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>

namespace Horizon::Engine
{
	class MeshAsset;
	class MaterialAsset;

	struct MeshDrawItem
	{
		const MeshAsset* pMesh = nullptr;
		const MaterialAsset* pMaterial = nullptr;
		RHI::GfxPipeline* pPipeline = nullptr;
		u32 submeshIndex = 0;
		Math::Mat4f model;
		Math::Vec3f worldPosition;

		u32 albedoTex = kInvalid32;
		u32 normalTex = kInvalid32;
		u32 metallicRoughnessTex = kInvalid32;
		u32 occlusionTex = kInvalid32;
		u32 emissiveTex = kInvalid32;
	};
}