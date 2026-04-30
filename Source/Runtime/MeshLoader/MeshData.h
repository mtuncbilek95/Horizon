#pragma once

#include <Runtime/Data/Geometry.h>
#include <Runtime/Data/Containers/Math.h>

#include <string>
#include <vector>

namespace Horizon
{
	struct MaterialData
	{
		std::string name;

		std::string albedoPath;
		std::string normalPath;
		std::string metallicRoughnessPath;
		std::string emissivePath;
		std::string occlusionPath;

		Math::Vec4f albedoFactor = { 1.f, 1.f, 1.f, 1.f };
		Math::Vec3f emissiveFactor = { 0.f, 0.f, 0.f };
		f32 metallicFactor = 0.f;
		f32 roughnessFactor = 1.f;
		f32 occlusionStrength = 1.f;
		b8 doubleSided = false;
	};

	struct MeshData
	{
		std::string name;
		Geometry geometry;
		u32 materialIndex = 0;
	};

	struct NodeData
	{
		std::string name;
		Math::Mat4f localTransform = Math::Mat4f(1.f);
		Math::Mat4f worldTransform = Math::Mat4f(1.f);
		std::vector<u32> meshIndices;
		std::vector<NodeData> children;
	};

	struct SceneData
	{
		std::vector<MeshData> meshes;
		std::vector<MaterialData> materials;
		NodeData rootNode;
	};
}
