#pragma once

#include <Engine/Asset/AssetFormat.h>
#include <Engine/Graphics/RHI/GfxDevice.h> 

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>

namespace Horizon
{
	struct MeshTag {};
	using MeshHandle = Handle<MeshTag>;
	struct MaterialTag {};
	using MaterialHandle = Handle<MaterialTag>;

	struct MeshAsset
	{
		GfxBufferHandle vertexBuffer;
		GfxBufferHandle indexBuffer;
		u32 vertexShaderView = ~0u;
		u32 indexCount = 0;
	};

	struct MaterialAsset
	{
		u32 texture[u32(Asset::MaterialSlot::Count)];
		glm::vec4 baseColorFactor{ 1.0f };
		f32 metallic = 1.0f;
		f32 roughness = 1.0f;
		glm::vec3 emissive{ 0.0f };
		f32 alphaCutoff = 0.5f;
		u32 flags = 0;
	};

	struct ModelInstance
	{
		MeshHandle mesh;
		MaterialHandle material;
		glm::mat4 transform;
	};

	struct Model
	{
		std::vector<ModelInstance> instances;
	};

	class AssetRegistry
	{
	public:
		const Model& LoadModel(const std::filesystem::path& hmodelPath);
		const MeshAsset& GetMesh(MeshHandle handle) const { return m_meshes[handle.index()]; }
		const MaterialAsset& GetMaterial(MaterialHandle handle) const { return m_materials[handle.index()]; }

		u32 MaterialBufferIndex() const { return m_materialBufferShaderView; }

	private:
		void RebuildMaterialBuffer();

		std::unordered_map<std::string, Model> m_cache;
		std::vector<MeshAsset> m_meshes;
		std::vector<MaterialAsset> m_materials;
		std::vector<GfxTextureHandle> m_textures;

		GfxBufferHandle m_materialBuffer;
		u32 m_materialBufferShaderView = ~0u;
	};

	inline AssetRegistry& AssetSystem()
	{
		static AssetRegistry instance;
		return instance;
	}
}