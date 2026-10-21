#include "MeshSystem.h"

#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Shader/ShaderAsset.h>
#include <Engine/Asset/Texture/TextureAsset.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	namespace
	{
		constexpr u64 sAlbedoTexHash = Reflect::Detail::Fnv1a("albedoTex");
		constexpr u64 sNormalTexHash = Reflect::Detail::Fnv1a("normalTex");
		constexpr u64 sMetallicRoughnessTexHash = Reflect::Detail::Fnv1a("metallicRoughnessTex");
		constexpr u64 sOcclusionTexHash = Reflect::Detail::Fnv1a("occlusionTex");
		constexpr u64 sEmissiveTexHash = Reflect::Detail::Fnv1a("emissiveTex");
	}

	b8 MeshSystem::OnInitialize()
	{
		m_assetService = GetEngine()->RequestService<AssetService>();
		if (m_assetService == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "AssetService is unavailable, meshes cannot be resolved");
			return false;
		}

		GraphicsContext* pGraphics = GetEngine()->RequestContext<GraphicsContext>();
		if (pGraphics == nullptr || pGraphics->GetDevice() == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, meshes cannot be unloaded safely");
			return false;
		}

		m_device = pGraphics->GetDevice();
		return true;
	}

	void MeshSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		if (m_lastScene != &currentScene)
		{
			UnloadSceneAssets();
			m_lastScene = &currentScene;
		}

		m_opaqueItems.Clear();
		m_maskedItems.Clear();
		m_translucentItems.Clear();
		m_drawListFrame = ctx.GetFrameIndex();

		currentScene.ForEach<MeshComponent, TransformComponent>([&](EntityHandle handl, MeshComponent& mesh, TransformComponent& transform)
			{
				const Guid& id = mesh.m_meshHandle.GetId();

				mesh.m_meshHandle.SetAsset(nullptr);

				if (!id.IsValid())
					return;

				if (!m_assetService->HasAsset(id))
				{
					if (mesh.m_resolvedId != id)
					{
						Terminal::Warn(StringOps::GetName(this), "{} is not registered yet, mesh stays hidden until it arrives", id.ToString());
						mesh.m_resolvedId = id;
					}

					return;
				}

				MeshAsset* pAsset = m_assetService->FindAsset<MeshAsset>(id);
				mesh.m_resolvedId = id;

				if (!pAsset)
					return;

				mesh.m_meshHandle.SetAsset(pAsset);
				pAsset->LoadAsync();
				m_loadedMeshes.insert(id);

				if (mesh.m_hideInRender || pAsset->GetResidency() != AssetResidency::Resident)
					return;

				const List<MeshAssetSubmesh>& subMeshes = pAsset->GetSubmeshes();

				for (u32 i = 0; i < (u32)subMeshes.GetCount(); ++i)
				{
					MeshDrawItem item = {};
					item.pMesh = pAsset;
					item.submeshIndex = i;
					item.model = transform.m_worldMatrix;
					item.worldPosition = transform.m_worldMatrix.GetTranslation();

					const MaterialBlendMode blendMode = ResolveMaterial(mesh, subMeshes[i].materialSlotIndex, item);

					if (blendMode == MaterialBlendMode::Masked)
						m_maskedItems.PushBack(item);
					else if (blendMode == MaterialBlendMode::Translucent)
						m_translucentItems.PushBack(item);
					else
						m_opaqueItems.PushBack(item);
				}
			});
	}

	void MeshSystem::OnFinalize()
	{
	}

	void MeshSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}

	MaterialBlendMode MeshSystem::ResolveMaterial(const MeshComponent& mesh, u32 slot, MeshDrawItem& outItem)
	{
		const Guid materialId = slot < mesh.m_materialOverrides.GetCount() ? mesh.m_materialOverrides[slot].GetId() : Guid();

		if (!materialId.IsValid())
			return MaterialBlendMode::Opaque;

		if (!m_assetService->HasAsset(materialId))
		{
			if (m_reportedAssets.insert(materialId).second)
				Terminal::Warn(StringOps::GetName(this), "{} material is not registered, fallback is used", materialId.ToString());

			return MaterialBlendMode::Opaque;
		}

		MaterialAsset* pMaterial = m_assetService->FindAsset<MaterialAsset>(materialId);
		if (!pMaterial)
			return MaterialBlendMode::Opaque;

		pMaterial->LoadSync();

		if (pMaterial->GetResidency() != AssetResidency::Resident)
		{
			if (m_reportedAssets.insert(materialId).second)
				Terminal::Warn(StringOps::GetName(this), "{} material could not be loaded, fallback is used", materialId.ToString());

			return MaterialBlendMode::Opaque;
		}

		if (m_loadedMaterials.insert(materialId).second)
		{
			for (const MaterialParamValue& value : pMaterial->GetValues())
			{
				if (value.type == MaterialParamType::Texture && value.textureId.IsValid())
					m_loadedTextures.insert(value.textureId);
			}
		}

		const Guid& shaderId = pMaterial->GetShaderId();

		if (!shaderId.IsValid())
			return MaterialBlendMode::Opaque;

		if (!m_assetService->HasAsset(shaderId))
		{
			if (m_reportedAssets.insert(shaderId).second)
				Terminal::Warn(StringOps::GetName(this), "{} shader is not registered, fallback is used", shaderId.ToString());

			return MaterialBlendMode::Opaque;
		}

		ShaderAsset* pShader = m_assetService->FindAsset<ShaderAsset>(shaderId);
		if (!pShader)
			return MaterialBlendMode::Opaque;

		pShader->LoadSync();
		m_loadedShaders.insert(shaderId);

		if (pShader->GetResidency() != AssetResidency::Resident || pShader->GetPipeline() == nullptr)
		{
			if (m_reportedAssets.insert(shaderId).second)
				Terminal::Warn(StringOps::GetName(this), "{} shader could not be loaded, fallback is used", shaderId.ToString());

			return MaterialBlendMode::Opaque;
		}

		outItem.pMaterial = pMaterial;
		outItem.pPipeline = pShader->GetPipeline();

		for (const MaterialParamValue& value : pMaterial->GetValues())
		{
			if (value.type != MaterialParamType::Texture || !value.textureId.IsValid() || !m_assetService->HasAsset(value.textureId))
				continue;

			TextureAsset* pTexture = m_assetService->FindAsset<TextureAsset>(value.textureId);
			if (!pTexture || pTexture->GetResidency() != AssetResidency::Resident)
				continue;

			const u32 shaderIndex = pTexture->GetShaderIndex();

			if (value.nameHash == sAlbedoTexHash)
				outItem.albedoTex = shaderIndex;
			else if (value.nameHash == sNormalTexHash)
				outItem.normalTex = shaderIndex;
			else if (value.nameHash == sMetallicRoughnessTexHash)
				outItem.metallicRoughnessTex = shaderIndex;
			else if (value.nameHash == sOcclusionTexHash)
				outItem.occlusionTex = shaderIndex;
			else if (value.nameHash == sEmissiveTexHash)
				outItem.emissiveTex = shaderIndex;
		}

		return pShader->GetRenderState().blendMode;
	}

	void MeshSystem::UnloadSceneAssets()
	{
		m_reportedAssets.clear();

		const usize total = m_loadedMeshes.size() + m_loadedMaterials.size() + m_loadedShaders.size() + m_loadedTextures.size();
		if (total == 0)
			return;

		m_device->WaitIdle();

		for (const Guid& id : m_loadedMeshes)
		{
			if (!m_assetService->HasAsset(id))
				continue;

			MeshAsset* pAsset = m_assetService->FindAsset<MeshAsset>(id);
			if (pAsset)
				pAsset->UnloadAsync();
		}

		for (const Guid& id : m_loadedMaterials)
		{
			if (!m_assetService->HasAsset(id))
				continue;

			MaterialAsset* pAsset = m_assetService->FindAsset<MaterialAsset>(id);
			if (pAsset)
				pAsset->UnloadSync();
		}

		for (const Guid& id : m_loadedShaders)
		{
			if (!m_assetService->HasAsset(id))
				continue;

			ShaderAsset* pAsset = m_assetService->FindAsset<ShaderAsset>(id);
			if (pAsset)
				pAsset->UnloadSync();
		}

		for (const Guid& id : m_loadedTextures)
		{
			if (!m_assetService->HasAsset(id))
				continue;

			TextureAsset* pAsset = m_assetService->FindAsset<TextureAsset>(id);
			if (pAsset)
				pAsset->UnloadSync();
		}

		Terminal::Info(StringOps::GetName(this), "{} meshes, {} materials, {} shaders and {} textures of the previous scene have been unloaded",
			m_loadedMeshes.size(), m_loadedMaterials.size(), m_loadedShaders.size(), m_loadedTextures.size());

		m_loadedMeshes.clear();
		m_loadedMaterials.clear();
		m_loadedShaders.clear();
		m_loadedTextures.clear();
	}
}