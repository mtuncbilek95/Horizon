#pragma once

namespace Horizon::Asset
{
	inline constexpr u32 ModelMagic = 0x4C444D48;
	inline constexpr u32 ModelVersion = 3;

	enum class MaterialSlot : u32
	{
		Albedo = 0,
		Normal,
		MetalRough,
		AmbientOcc,
		Emissive,
		Count
	};

	enum class MaterialFlag : u32
	{
		DoubleSided = 1 << 0,
		AlphaMask = 1 << 1
	};
	GENERATE_FLAGS(MaterialFlag);

	struct MeshRecord
	{
		u32 vertexOffset, vertexCount;
		u32 indexOffset, indexCount;
		u32 materialIndex;
		f32 boundsCenter[3];
		f32 boundsRadius;
	};

	struct MaterialRecord
	{
		i32 texture[i32(MaterialSlot::Count)];
		f32 baseColorFactor[4];
		f32 metallicFactor, roughnessFactor;
		f32 emissiveFactor[3];
		f32 alphaCutoff;
		MaterialFlag flags;
	};

	struct InstanceRecord 
	{ 
		u32 meshIndex; 
		f32 transform[16]; 
	};

	struct TextureRecord
	{
		char path[256];
		u32 width, height, format, dataOffset, dataSize;
	};

	struct CookedVertex
	{
		f32 position[3], normal[3], tangent[4], texCoord[2];
	};

	struct ModelHeader
	{
		u32 magic, version;
		u32 meshCount, materialCount, instanceCount, textureCount;
		u32 vertexCount, indexCount;
		u32 vertexStride;
		u32 textureDataSize;
	};

	static_assert(sizeof(MeshRecord) == 36);
	static_assert(sizeof(MaterialRecord) == 64);
	static_assert(sizeof(InstanceRecord) == 68);
	static_assert(sizeof(TextureRecord) == 276);
	static_assert(sizeof(CookedVertex) == 48);
	static_assert(sizeof(ModelHeader) == 40);
}