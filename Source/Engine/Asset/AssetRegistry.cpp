#include <Engine/Asset/AssetRegistry.h>

#include <Engine/Graphics/RHI/GfxBuffer.h>
#include <Engine/Graphics/RHI/GfxTexture.h>
#include <Engine/Log/Log.h>

#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <algorithm>

namespace Horizon
{
	using namespace Asset;

	struct GPUMaterial
	{
		u32 albedo, normal, metalRough, ambientOcclusion;
		u32 emissive, flags;
		f32 alphaCutoff;
		u32 _pad0;
		glm::vec4 baseColorFactor;
		glm::vec3 emissiveFactor;
		f32 metallic;
		f32 roughness;
		glm::vec3 _pad1;
	};
	static_assert(sizeof(GPUMaterial) == 80);

	static std::vector<u8> DownsampleRGBA8(const u8* sourceData, u32 width, u32 height, u32& outWidth, u32& outHeight)
	{
		outWidth = width > 1 ? width / 2 : 1;
		outHeight = height > 1 ? height / 2 : 1;
		std::vector<u8> destData((usize)outWidth * outHeight * 4);
		for (u32 rowIdx = 0; rowIdx < outHeight; rowIdx++)
		{
			u32 rowTop = rowIdx * 2, rowBottom = std::min(rowTop + 1, height - 1);
			for (u32 colIdx = 0; colIdx < outWidth; colIdx++)
			{
				u32 colLeft = colIdx * 2, colRight = std::min(colLeft + 1, width - 1);
				const u8* topLeft = sourceData + ((usize)rowTop * width + colLeft) * 4;
				const u8* topRight = sourceData + ((usize)rowTop * width + colRight) * 4;
				const u8* bottomLeft = sourceData + ((usize)rowBottom * width + colLeft) * 4;
				const u8* bottomRight = sourceData + ((usize)rowBottom * width + colRight) * 4;
				u8* outPixel = destData.data() + ((usize)rowIdx * outWidth + colIdx) * 4;
				for (u32 channelIdx = 0; channelIdx < 4; channelIdx++)
					outPixel[channelIdx] = (u8)((topLeft[channelIdx] + topRight[channelIdx] + bottomLeft[channelIdx] + bottomRight[channelIdx] + 2) / 4);
			}
		}
		return destData;
	}

	void AssetRegistry::RebuildMaterialBuffer()
	{
		if (m_materials.empty())
			return;

		std::vector<GPUMaterial> gpuMaterials(m_materials.size());
		for (usize materialIdx = 0; materialIdx < m_materials.size(); materialIdx++)
		{
			const MaterialAsset& materialAsset = m_materials[materialIdx];
			GPUMaterial& gpuMaterial = gpuMaterials[materialIdx];

			gpuMaterial.albedo = materialAsset.texture[u32(MaterialSlot::Albedo)];
			gpuMaterial.normal = materialAsset.texture[u32(MaterialSlot::Normal)];
			gpuMaterial.metalRough = materialAsset.texture[u32(MaterialSlot::MetalRough)];
			gpuMaterial.ambientOcclusion = materialAsset.texture[u32(MaterialSlot::AmbientOcc)];
			gpuMaterial.emissive = materialAsset.texture[u32(MaterialSlot::Emissive)];
			gpuMaterial.flags = materialAsset.flags;
			gpuMaterial.alphaCutoff = materialAsset.alphaCutoff;
			gpuMaterial.baseColorFactor = materialAsset.baseColorFactor;
			gpuMaterial.emissiveFactor = materialAsset.emissive;
			gpuMaterial.metallic = materialAsset.metallic;
			gpuMaterial.roughness = materialAsset.roughness;
		}

		if (m_materialBufferShaderView != ~0u)
			GfxDevice::DestroyBuffer(m_materialBuffer);

		GfxBufferDesc bufferDesc{};
		bufferDesc.size = gpuMaterials.size() * sizeof(GPUMaterial);
		bufferDesc.stride = sizeof(GPUMaterial);
		bufferDesc.usage = GfxBufferUsage::Storage;
		bufferDesc.memory = GfxMemoryType::GPU;
		m_materialBuffer = GfxDevice::CreateBuffer(bufferDesc);

		GfxDevice::UploadBuffer(m_materialBuffer, gpuMaterials.data(), bufferDesc.size);

		m_materialBufferShaderView = GfxDevice::BindlessShaderView(m_materialBuffer);
	}

	const Model& AssetRegistry::LoadModel(const std::filesystem::path& path)
	{
		std::string key = path.string();

		auto cacheEntry = m_cache.find(key);
		if (cacheEntry != m_cache.end())
			return cacheEntry->second;

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			MainLog::Error("hmodel could not be opened: {}", key);
			return m_cache[key];
		}
		usize size = (usize)file.tellg();
		std::vector<u8> bytes(size);
		file.seekg(0);
		file.read((char*)bytes.data(), size);

		const ModelHeader* header = (const ModelHeader*)bytes.data();
		if (header->magic != ModelMagic || header->version != ModelVersion)
		{
			MainLog::Error("hmodel invalid/old (magic/version): {}", key);
			return m_cache[key];
		}

		const MeshRecord* meshRecords = (const MeshRecord*)(header + 1);
		const MaterialRecord* materialRecords = (const MaterialRecord*)(meshRecords + header->meshCount);
		const InstanceRecord* instanceRecords = (const InstanceRecord*)(materialRecords + header->materialCount);
		const TextureRecord* textureRecords = (const TextureRecord*)(instanceRecords + header->instanceCount);
		const CookedVertex* vertices = (const CookedVertex*)(textureRecords + header->textureCount);
		const u32* indices = (const u32*)(vertices + header->vertexCount);
		const u8* textureBlob = (const u8*)(indices + header->indexCount);

		std::vector<u32> textureBindless(header->textureCount, ~0u);
		for (u32 textureIdx = 0; textureIdx < header->textureCount; textureIdx++)
		{
			const TextureRecord& textureRecord = textureRecords[textureIdx];

			u32 mipCount = 1;
			for (u32 mipWidth = textureRecord.width, mipHeight = textureRecord.height; mipWidth > 1 || mipHeight > 1; mipCount++)
			{
				mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
				mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
			}

			GfxTextureDesc textureDesc = {};
			textureDesc.width = textureRecord.width;
			textureDesc.height = textureRecord.height;
			textureDesc.mipLevels = mipCount;
			textureDesc.format = GfxTextureFormat::RGBA8;
			textureDesc.usage = GfxTextureUsage::Sampled;

			GfxTextureHandle texture = GfxDevice::CreateTexture(textureDesc);
			GfxDevice::UploadTexture(texture, textureBlob + textureRecord.dataOffset, 0);

			std::vector<u8> currentMip(textureBlob + textureRecord.dataOffset, textureBlob + textureRecord.dataOffset + (usize)textureRecord.width * textureRecord.height * 4);
			u32 currentWidth = textureRecord.width, currentHeight = textureRecord.height;
			for (u32 mipIdx = 1; mipIdx < mipCount; mipIdx++)
			{
				u32 newWidth, newHeight;
				std::vector<u8> nextMip = DownsampleRGBA8(currentMip.data(), currentWidth, currentHeight, newWidth, newHeight);
				GfxDevice::UploadTexture(texture, nextMip.data(), mipIdx);
				currentMip.swap(nextMip);
				currentWidth = newWidth;
				currentHeight = newHeight;
			}

			textureBindless[textureIdx] = GfxDevice::BindlessShaderView(texture);
			m_textures.push_back(texture);
		}

		std::vector<MeshHandle> meshHandles(header->meshCount);
		for (u32 meshIdx = 0; meshIdx < header->meshCount; meshIdx++)
		{
			const MeshRecord& meshRecord = meshRecords[meshIdx];

			u64 vertexBytes = (u64)meshRecord.vertexCount * sizeof(CookedVertex);
			u64 indexBytes = (u64)meshRecord.indexCount * sizeof(u32);

			GfxBufferDesc vertexDesc{};
			vertexDesc.size = vertexBytes;
			vertexDesc.usage = GfxBufferUsage::Storage;
			vertexDesc.memory = GfxMemoryType::GPU;
			vertexDesc.stride = sizeof(CookedVertex);
			GfxBufferHandle vertexBuffer = GfxDevice::CreateBuffer(vertexDesc);
			GfxDevice::UploadBuffer(vertexBuffer, vertices + meshRecord.vertexOffset, vertexBytes);

			GfxBufferDesc indexDesc{};
			indexDesc.size = indexBytes;
			indexDesc.usage = GfxBufferUsage::Index;
			indexDesc.memory = GfxMemoryType::GPU;
			GfxBufferHandle indexBuffer = GfxDevice::CreateBuffer(indexDesc);
			GfxDevice::UploadBuffer(indexBuffer, indices + meshRecord.indexOffset, indexBytes);

			MeshAsset asset{};
			asset.vertexBuffer = vertexBuffer;
			asset.indexBuffer = indexBuffer;
			asset.vertexShaderView = GfxDevice::BindlessShaderView(vertexBuffer);
			asset.indexCount = meshRecord.indexCount;

			meshHandles[meshIdx] = MeshHandle::make((u32)m_meshes.size(), 0);
			m_meshes.push_back(asset);
		}

		std::vector<MaterialHandle> materialHandles(header->materialCount);
		for (u32 materialIdx = 0; materialIdx < header->materialCount; materialIdx++)
		{
			const MaterialRecord& materialRecord = materialRecords[materialIdx];

			MaterialAsset asset{};
			for (u32 slotIdx = 0; slotIdx < (u32)MaterialSlot::Count; slotIdx++)
				asset.texture[slotIdx] = (materialRecord.texture[slotIdx] >= 0) ? textureBindless[materialRecord.texture[slotIdx]] : ~0u;

			asset.baseColorFactor = glm::make_vec4(materialRecord.baseColorFactor);
			asset.metallic = materialRecord.metallicFactor;
			asset.roughness = materialRecord.roughnessFactor;
			asset.emissive = glm::make_vec3(materialRecord.emissiveFactor);
			asset.alphaCutoff = materialRecord.alphaCutoff;
			asset.flags = (u32)materialRecord.flags;

			materialHandles[materialIdx] = MaterialHandle::make((u32)m_materials.size(), 0);
			m_materials.push_back(asset);
		}

		Model loadedModel;
		loadedModel.instances.reserve(header->instanceCount);
		for (u32 instanceIdx = 0; instanceIdx < header->instanceCount; instanceIdx++)
		{
			const InstanceRecord& instanceRecord = instanceRecords[instanceIdx];
			const MeshRecord& meshRecord = meshRecords[instanceRecord.meshIndex];

			ModelInstance instance{};
			instance.mesh = meshHandles[instanceRecord.meshIndex];
			instance.material = materialHandles[meshRecord.materialIndex];
			instance.transform = glm::make_mat4(instanceRecord.transform);
			loadedModel.instances.push_back(instance);
		}

		RebuildMaterialBuffer();

		auto [inserted, _] = m_cache.emplace(std::move(key), std::move(loadedModel));
		return inserted->second;
	}
}
