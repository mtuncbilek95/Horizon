#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <vector>
#include <span>

namespace Horizon
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 binormal;
		glm::vec4 color;
		glm::vec2 texCoord;
	};

	enum class VertexRawData : u32
	{
		None = 0,
		Position = 1 << 0,
		Normal = 1 << 1,
		Tangent = 1 << 2,
		Bitangent = 1 << 3,
		Color = 1 << 4,
		TexCoord = 1 << 5,
	};
	GENERATE_FLAGS(VertexRawData);

	class Geometry final
	{
	public:
		Geometry() = default;
		Geometry(u32 vertexCount, u32 indexCount);

		void AddPosition(glm::vec3 position, u32 index);
		void AddNormal(glm::vec3 normal, u32 index);
		void AddTangent(glm::vec3 tangent, u32 index);
		void AddBinormal(glm::vec3 binormal, u32 index);
		void AddColor(glm::vec4 color, u32 index);
		void AddTexCoord(glm::vec2 texCoord, u32 index);
		void AddIndex(u32 indexValue, u32 slotIndex);
		void AddIndex(std::vector<u32>&& indices);

		std::vector<uint8_t> GenerateRawVertex(VertexRawData flags) const;
		std::vector<uint8_t> GenerateRawIndex() const;

		inline const std::vector<Vertex>& GetVertexData() const { return m_vertices; }
		inline std::vector<u32>& GetIndexData() { return m_indices; }

		inline size_t GetVertexCount() const { return m_vertices.size(); }
		inline size_t GetIndexCount() const { return m_indices.size(); }

		u32 GetVertexStride() const { return m_stride; }

	private:
		std::vector<Vertex> m_vertices;
		std::vector<u32> m_indices;
		mutable u32 m_stride = 0;
	};
}
