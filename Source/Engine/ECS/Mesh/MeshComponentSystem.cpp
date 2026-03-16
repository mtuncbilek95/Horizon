#include "MeshComponentSystem.h"

#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/RHI/Shader/GfxShader.h>

#include <Runtime/ShaderCompiler/ShaderCompiler.h>

#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/ECS/Mesh/MeshComponent.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <queue>
#include <map>
#include <numeric>

namespace Horizon
{
	struct Vertex {
		Math::Vec4f position;
		Math::Vec4f normal;
		Math::Vec4f tangent;
		Math::Vec2f uv;
		Math::Vec2f padding;
	};

	struct Meshlet
	{
		u32 vertices[64];
		u32 indices[378];
		u32 vertexCount;
		u32 indexCount;
	};

	struct MeshletBounds
	{
		float cx, cy, cz;
		float radius;
	};

	struct MeshletLODInfo
	{
		u32 lod1MeshletID;
		u32 lod2MeshletID;
		u32 isLOD1Owner;
		u32 isLOD2Owner;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<u32>    indices;
	};

	MeshData LoadAssimp(const std::filesystem::path& filePath)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filePath.string(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace
		);

		assert(scene && scene->mNumMeshes > 0);

		MeshData result;
		const aiMesh* mesh = scene->mMeshes[0];

		result.vertices.reserve(mesh->mNumVertices);
		for (u32 i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex v = {};
			v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f };
			if (mesh->HasNormals())
				v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f };
			if (mesh->HasTangentsAndBitangents())
				v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f };
			if (mesh->HasTextureCoords(0))
				v.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			result.vertices.push_back(v);
		}

		result.indices.reserve(mesh->mNumFaces * 3);
		for (u32 i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			result.indices.push_back(face.mIndices[0]);
			result.indices.push_back(face.mIndices[1]);
			result.indices.push_back(face.mIndices[2]);
		}

		return result;
	}

	std::vector<Meshlet> BuildMeshlets(const std::vector<Vertex>& vertices, const std::vector<u32>& indices)
	{
		u32 triCount = indices.size() / 3;

		struct Vec3Key {
			float x, y, z;
			bool operator<(const Vec3Key& o) const {
				if (x != o.x) return x < o.x;
				if (y != o.y) return y < o.y;
				return z < o.z;
			}
		};

		std::map<Vec3Key, std::vector<u32>> posToTriangles;
		for (u32 i = 0; i < triCount; i++)
			for (u32 j = 0; j < 3; j++)
			{
				u32 idx = indices[i * 3 + j];
				Vec3Key key = { vertices[idx].position.x, vertices[idx].position.y, vertices[idx].position.z };
				auto& list = posToTriangles[key];
				if (std::find(list.begin(), list.end(), i) == list.end())
					list.push_back(i);
			}

		std::vector<std::vector<u32>> triNeighbors(triCount);
		for (auto& [pos, tris] : posToTriangles)
			for (u32 a : tris)
				for (u32 b : tris)
					if (a != b)
					{
						auto& list = triNeighbors[a];
						if (std::find(list.begin(), list.end(), b) == list.end())
							list.push_back(b);
					}

		std::vector<bool>    triUsed(triCount, false);
		std::vector<Meshlet> meshlets;

		u32 startTri = 0;
		while (startTri < triCount)
		{
			while (startTri < triCount && triUsed[startTri]) startTri++;
			if (startTri >= triCount) break;

			Meshlet current = {};
			std::unordered_map<u32, u32> vertexMap;
			std::queue<u32> frontier;
			frontier.push(startTri);

			while (!frontier.empty())
			{
				u32 triIdx = frontier.front();
				frontier.pop();

				if (triUsed[triIdx]) continue;

				u32 tv[3] = {
					indices[triIdx * 3 + 0],
					indices[triIdx * 3 + 1],
					indices[triIdx * 3 + 2]
				};

				u32 newVerts = 0;
				for (u32 v : tv)
					if (vertexMap.find(v) == vertexMap.end())
						newVerts++;

				bool vertexLimitHit = (current.vertexCount + newVerts) > 64;
				bool triangleLimitHit = (current.indexCount / 3) >= 126;

				if (vertexLimitHit || triangleLimitHit) continue;

				triUsed[triIdx] = true;

				for (u32 v : tv)
				{
					if (vertexMap.find(v) == vertexMap.end())
					{
						vertexMap[v] = current.vertexCount;
						current.vertices[current.vertexCount++] = v;
					}
					current.indices[current.indexCount++] = vertexMap[v];
				}

				for (u32 neighborTri : triNeighbors[triIdx])
					if (!triUsed[neighborTri])
						frontier.push(neighborTri);
			}

			if (current.indexCount > 0)
				meshlets.push_back(current);
		}

		return meshlets;
	}

	std::vector<u32> SimplifyMesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, float targetRatio)
	{
		u32 targetTriCount = std::max((u32)(indices.size() / 3 * targetRatio), 4u);

		struct Edge { u32 v0, v1; float cost; };
		std::vector<Edge> edges;

		for (u32 i = 0; i < indices.size(); i += 3)
		{
			u32 a = indices[i + 0], b = indices[i + 1], c = indices[i + 2];
			auto addEdge = [&](u32 x, u32 y)
				{
					float dx = vertices[x].position.x - vertices[y].position.x;
					float dy = vertices[x].position.y - vertices[y].position.y;
					float dz = vertices[x].position.z - vertices[y].position.z;
					edges.push_back({ x, y, dx * dx + dy * dy + dz * dz });
				};
			addEdge(a, b); addEdge(b, c); addEdge(c, a);
		}

		std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) { return a.cost < b.cost; });

		std::vector<u32> remap(vertices.size());
		std::iota(remap.begin(), remap.end(), 0);

		auto find = [&](u32 v) -> u32 {
			while (remap[v] != v)
			{
				remap[v] = remap[remap[v]];
				v = remap[v];
			}
			return v;
			};

		u32 currentTriCount = indices.size() / 3;
		for (const Edge& e : edges)
		{
			if (currentTriCount <= targetTriCount) break;
			u32 v0 = find(e.v0), v1 = find(e.v1);
			if (v0 == v1) continue;
			remap[v1] = v0;
			currentTriCount--;
		}

		std::vector<u32> newIndices;
		for (u32 i = 0; i < indices.size(); i += 3)
		{
			u32 a = find(indices[i + 0]);
			u32 b = find(indices[i + 1]);
			u32 c = find(indices[i + 2]);
			if (a == b || b == c || a == c) continue;
			newIndices.push_back(a);
			newIndices.push_back(b);
			newIndices.push_back(c);
		}

		if (newIndices.empty())
			return indices;

		return newIndices;
	}

	MeshletBounds ComputeMeshletBounds(const Meshlet& meshlet, const std::vector<Vertex>& vertices)
	{
		MeshletBounds bounds = {};

		for (u32 i = 0; i < meshlet.vertexCount; i++)
		{
			u32 vi = meshlet.vertices[i];
			bounds.cx += vertices[vi].position.x;
			bounds.cy += vertices[vi].position.y;
			bounds.cz += vertices[vi].position.z;
		}
		bounds.cx /= meshlet.vertexCount;
		bounds.cy /= meshlet.vertexCount;
		bounds.cz /= meshlet.vertexCount;

		for (u32 i = 0; i < meshlet.vertexCount; i++)
		{
			u32 vi = meshlet.vertices[i];
			float dx = vertices[vi].position.x - bounds.cx;
			float dy = vertices[vi].position.y - bounds.cy;
			float dz = vertices[vi].position.z - bounds.cz;
			bounds.radius = std::max(bounds.radius, std::sqrt(dx * dx + dy * dy + dz * dz));
		}

		return bounds;
	}

	std::vector<MeshletLODInfo> BuildLODMapping(const std::vector<Meshlet>& lod0, const std::vector<Meshlet>& lod1,
		const std::vector<Meshlet>& lod2, const std::vector<Vertex>& vertices)
	{
		auto centerOf = [&](const Meshlet& m) -> Math::Vec3f
			{
				float cx = 0, cy = 0, cz = 0;
				for (u32 i = 0; i < m.vertexCount; i++)
				{
					u32 vi = m.vertices[i];
					cx += vertices[vi].position.x;
					cy += vertices[vi].position.y;
					cz += vertices[vi].position.z;
				}
				return { cx / m.vertexCount, cy / m.vertexCount, cz / m.vertexCount };
			};

		auto findNearest = [&](const Math::Vec3f& c, const std::vector<Meshlet>& lod) -> u32
			{
				float bestDist = FLT_MAX;
				u32   bestIdx = 0;
				for (u32 j = 0; j < lod.size(); j++)
				{
					Math::Vec3f lc = centerOf(lod[j]);
					float dx = c.x - lc.x, dy = c.y - lc.y, dz = c.z - lc.z;
					float dist = dx * dx + dy * dy + dz * dz;
					if (dist < bestDist) { bestDist = dist; bestIdx = j; }
				}
				return bestIdx;
			};

		std::vector<MeshletLODInfo> mapping(lod0.size());
		std::vector<bool> lod1Owned(lod1.size(), false);
		std::vector<bool> lod2Owned(lod2.size(), false);

		for (u32 i = 0; i < lod0.size(); i++)
		{
			Math::Vec3f c = centerOf(lod0[i]);
			u32 l1 = findNearest(c, lod1);
			u32 l2 = findNearest(c, lod2);

			mapping[i].lod1MeshletID = l1;
			mapping[i].lod2MeshletID = l2;

			if (!lod1Owned[l1]) { lod1Owned[l1] = true; mapping[i].isLOD1Owner = 1; }
			else                  mapping[i].isLOD1Owner = 0;

			if (!lod2Owned[l2]) { lod2Owned[l2] = true; mapping[i].isLOD2Owner = 1; }
			else                  mapping[i].isLOD2Owner = 0;
		}

		return mapping;
	}

	std::shared_ptr<GfxBuffer> m_meshletBufferLOD0;
	std::shared_ptr<GfxBuffer> m_meshletBufferLOD1;
	std::shared_ptr<GfxBuffer> m_meshletBufferLOD2;
	std::shared_ptr<GfxBuffer> m_vertexBuffer;
	std::shared_ptr<GfxBuffer> m_meshletBoundsBuffer;
	std::shared_ptr<GfxBuffer> m_lodMappingBuffer;

	std::shared_ptr<GfxDescriptorLayout> m_meshDescriptorLayout;
	std::shared_ptr<GfxDescriptorBuffer> m_meshDescriptorBuffer;
	std::shared_ptr<GfxPipeline>         m_meshPipeline;

	std::shared_ptr<GfxShader> m_taskShader;
	std::shared_ptr<GfxShader> m_meshShader;
	std::shared_ptr<GfxShader> m_fragShader;

	std::vector<Meshlet>       m_meshletsLOD0;
	std::vector<Meshlet>       m_meshletsLOD1;
	std::vector<Meshlet>       m_meshletsLOD2;
	std::vector<MeshletBounds> m_boundsLOD0;
	std::vector<MeshletLODInfo> m_lodMapping;

	SystemReport MeshComponentSystem::OnStart()
	{
		MeshData meshData = LoadAssimp("../assets/Suzanne.fbx");

		std::vector<u32> indicesLOD0 = meshData.indices;
		std::vector<u32> indicesLOD1 = SimplifyMesh(meshData.vertices, meshData.indices, 0.90f);
		std::vector<u32> indicesLOD2 = SimplifyMesh(meshData.vertices, meshData.indices, 0.75f);

		m_meshletsLOD0 = BuildMeshlets(meshData.vertices, indicesLOD0);
		m_meshletsLOD1 = BuildMeshlets(meshData.vertices, indicesLOD1);
		m_meshletsLOD2 = BuildMeshlets(meshData.vertices, indicesLOD2);

		for (const Meshlet& m : m_meshletsLOD0)
			m_boundsLOD0.push_back(ComputeMeshletBounds(m, meshData.vertices));

		m_lodMapping = BuildLODMapping(m_meshletsLOD0, m_meshletsLOD1, m_meshletsLOD2, meshData.vertices);

		auto& graphicsSystem = RequestSystem<GraphicsSystem>();
		auto* gDevice = graphicsSystem.Device();

		assert(m_meshletsLOD0.size() > 0 && "LOD0 meshlets empty");
		m_meshletBufferLOD0 = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(Meshlet) * m_meshletsLOD0.size())
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_meshletBufferLOD0->Update(m_meshletsLOD0.data(), sizeof(Meshlet) * m_meshletsLOD0.size());

		assert(m_meshletsLOD1.size() > 0 && "LOD1 meshlets empty");
		m_meshletBufferLOD1 = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(Meshlet) * m_meshletsLOD1.size())
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_meshletBufferLOD1->Update(m_meshletsLOD1.data(), sizeof(Meshlet) * m_meshletsLOD1.size());

		assert(m_meshletsLOD2.size() > 0 && "LOD2 meshlets empty");
		m_meshletBufferLOD2 = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(Meshlet) * m_meshletsLOD2.size())
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_meshletBufferLOD2->Update(m_meshletsLOD2.data(), sizeof(Meshlet) * m_meshletsLOD2.size());

		assert(meshData.vertices.size() > 0 && "Vertices empty");
		m_vertexBuffer = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(Vertex) * meshData.vertices.size())
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_vertexBuffer->Update(meshData.vertices.data(), sizeof(Vertex) * meshData.vertices.size());

		assert(m_boundsLOD0.size() > 0 && "Bounds empty");
		m_meshletBoundsBuffer = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(MeshletBounds) * m_boundsLOD0.size())
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_meshletBoundsBuffer->Update(m_boundsLOD0.data(), sizeof(MeshletBounds) * m_boundsLOD0.size());

		assert(m_lodMapping.size() > 0 && "LOD mapping empty");
		m_lodMappingBuffer = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(MeshletLODInfo) * m_lodMapping.size())
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_lodMappingBuffer->Update(m_lodMapping.data(), sizeof(MeshletLODInfo) * m_lodMapping.size());

		m_meshDescriptorLayout = gDevice->CreateDescriptorLayout(
			GfxDescriptorLayoutDesc()
			.addBinding(DescriptorBinding().setBinding(0).setType(DescriptorType::Storage).setStage(ShaderStage::Mesh | ShaderStage::Task).setCount(1))
			.addBinding(DescriptorBinding().setBinding(1).setType(DescriptorType::Storage).setStage(ShaderStage::Mesh).setCount(1))
			.addBinding(DescriptorBinding().setBinding(2).setType(DescriptorType::Storage).setStage(ShaderStage::Mesh).setCount(1))
			.addBinding(DescriptorBinding().setBinding(3).setType(DescriptorType::Storage).setStage(ShaderStage::Mesh).setCount(1))
			.addBinding(DescriptorBinding().setBinding(4).setType(DescriptorType::Storage).setStage(ShaderStage::Task).setCount(1))
			.addBinding(DescriptorBinding().setBinding(5).setType(DescriptorType::Storage).setStage(ShaderStage::Task).setCount(1))
			.setFlags(DescriptorLayoutFlags::Buffer)
		);

		m_meshDescriptorBuffer = gDevice->CreateDescriptorBuffer(
			GfxDescriptorBufferDesc()
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setMemoryUsage(MemoryUsage::AutoPreferDevice)
			.setSize(m_meshDescriptorLayout->SizeInBytes())
			.setType(DescriptorBufferType::Resource)
			.setUsage(DescriptorType::Storage)
		);

		m_meshDescriptorBuffer->RequestPayload(m_meshletBufferLOD0.get(), m_meshDescriptorLayout->BindingOffset(0));
		m_meshDescriptorBuffer->RequestPayload(m_meshletBufferLOD1.get(), m_meshDescriptorLayout->BindingOffset(1));
		m_meshDescriptorBuffer->RequestPayload(m_meshletBufferLOD2.get(), m_meshDescriptorLayout->BindingOffset(2));
		m_meshDescriptorBuffer->RequestPayload(m_vertexBuffer.get(), m_meshDescriptorLayout->BindingOffset(3));
		m_meshDescriptorBuffer->RequestPayload(m_meshletBoundsBuffer.get(), m_meshDescriptorLayout->BindingOffset(4));
		m_meshDescriptorBuffer->RequestPayload(m_lodMappingBuffer.get(), m_meshDescriptorLayout->BindingOffset(5));

		std::string taskCode = DataReader::ReadToTextFile("../shaders/Geometry.task");
		std::string meshCode = DataReader::ReadToTextFile("../shaders/Geometry.mesh");
		std::string fragCode = DataReader::ReadToTextFile("../shaders/Geometry.frag");

		ReadArray<u32> taskSpirv = ShaderCompiler::GenerateSpirv(taskCode, "main", ShaderStage::Task);
		ReadArray<u32> meshSpirv = ShaderCompiler::GenerateSpirv(meshCode, "main", ShaderStage::Mesh);
		ReadArray<u32> fragSpirv = ShaderCompiler::GenerateSpirv(fragCode, "main", ShaderStage::Fragment);

		m_taskShader = gDevice->CreateShader(GfxShaderDesc().setStage(ShaderStage::Task).setByteCode(*taskSpirv));
		m_meshShader = gDevice->CreateShader(GfxShaderDesc().setStage(ShaderStage::Mesh).setByteCode(*meshSpirv));
		m_fragShader = gDevice->CreateShader(GfxShaderDesc().setStage(ShaderStage::Fragment).setByteCode(*fragSpirv));

		m_meshPipeline = gDevice->CreateGraphicsPipeline(
			GfxGraphicsPipelineDesc()
			.addLayout(m_meshDescriptorLayout.get())
			.addShader(m_taskShader.get())
			.addShader(m_meshShader.get())
			.addShader(m_fragShader.get())
			.setRasterizer(
				RasterizerState()
				.setCullMode(CullMode::Back)
				.setFaceOrientation(FaceOrientation::CCW)
				.setPolygonMode(PolygonMode::Fill)
				.setDepthBiasEnable(false))
			.setPushConstants(
				PushConstants()
				.addRange(
					PushConstantRange()
					.setSizeOfData(160u)
					.setStage(ShaderStage::Task | ShaderStage::Mesh)
					.setOffset(0)))
			.setInput(InputAssembler().setTopology(PrimitiveTopology::TriangleList))
			.setBlend(
				BlendState()
				.setLogicEnable(false)
				.addAttachment(BlendAttachment().setBlendEnable(false).setColorMask(ColorComponent::All)))
			.setDepthStencil(
				DepthStencilState()
				.setDepthTestEnable(false)
				.setDepthWriteEnable(false)
				.setDepthOp(CompareOp::Less))
			.setViewport(Viewport().setSize({ 1920.f, 1080.f }))
			.setScissor(Scissor().setExtent({ 1920, 1080 }))
			.setRenderPass(graphicsSystem.PresentationPass())
			.setFlags(PipelineFlags::DescriptorBuffer)
		);

		return SystemReport();
	}

	void MeshComponentSystem::OnTick()
	{
		auto& graphicsSystem = RequestSystem<GraphicsSystem>();

		graphicsSystem.Submit({ [](GfxCommandBuffer* cmd)
		{
			cmd->BindPipeline(m_meshPipeline.get());
			cmd->BindDescriptorBuffer(m_meshPipeline.get(), ShaderStage::Task | ShaderStage::Mesh, 0, m_meshDescriptorBuffer.get());
			cmd->DrawMeshTask(m_meshletsLOD0.size(), 1, 1);
		} });
	}

	void MeshComponentSystem::OnStop()
	{
	}
}