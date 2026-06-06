#include "Geometry.h"

#include <cstring>

namespace Horizon
{
	Geometry::Geometry(u32 vertexCount, u32 indexCount)
		: m_vertices(vertexCount), m_indices(indexCount)
	{}

	void Geometry::AddPosition(glm::vec3 position, u32 index)
	{
		if (index >= m_vertices.size())
			m_vertices.resize(index + 1);

		m_vertices[index].position = position;
	}

	void Geometry::AddNormal(glm::vec3 normal, u32 index)
	{
		if (index >= m_vertices.size())
			m_vertices.resize(index + 1);

		m_vertices[index].normal = normal;
	}

	void Geometry::AddTangent(glm::vec3 tangent, u32 index)
	{
		if (index >= m_vertices.size())
			m_vertices.resize(index + 1);

		m_vertices[index].tangent = tangent;
	}

	void Geometry::AddBinormal(glm::vec3 binormal, u32 index)
	{
		if (index >= m_vertices.size())
			m_vertices.resize(index + 1);

		m_vertices[index].binormal = binormal;
	}

	void Geometry::AddColor(glm::vec4 color, u32 index)
	{
		if (index >= m_vertices.size())
			m_vertices.resize(index + 1);

		m_vertices[index].color = color;
	}

	void Geometry::AddTexCoord(glm::vec2 texCoord, u32 index)
	{
		if (index >= m_vertices.size())
			m_vertices.resize(index + 1);

		m_vertices[index].texCoord = texCoord;
	}

	void Geometry::AddIndex(u32 indexValue, u32 slotIndex)
	{
		if (slotIndex >= m_indices.size())
			m_indices.resize(slotIndex + 1);

		m_indices[slotIndex] = indexValue;
	}

	void Geometry::AddIndex(std::vector<u32>&& indices)
	{
		m_indices = std::move(indices);
	}

	std::vector<uint8_t> Geometry::GenerateRawVertex(VertexRawData flags) const
	{
		if (flags == VertexRawData::None)
			return {};

		size_t vertexStride = 0;
		if (has(flags, VertexRawData::Position))
			vertexStride += sizeof(glm::vec3);
		if (has(flags, VertexRawData::Normal))
			vertexStride += sizeof(glm::vec3);
		if (has(flags, VertexRawData::Tangent))
			vertexStride += sizeof(glm::vec3);
		if (has(flags, VertexRawData::Bitangent))
			vertexStride += sizeof(glm::vec3);
		if (has(flags, VertexRawData::Color))
			vertexStride += sizeof(glm::vec4);
		if (has(flags, VertexRawData::TexCoord))
			vertexStride += sizeof(glm::vec2);

		m_stride = u32(vertexStride);

		std::vector<uint8_t> returnData(vertexStride * m_vertices.size());
		uint8_t* writePtr = returnData.data();

		for (const auto& vertex : m_vertices)
		{
			if (has(flags, VertexRawData::Position))
			{
				std::memcpy(writePtr, &vertex.position, sizeof(glm::vec3));
				writePtr += sizeof(glm::vec3);
			}
			if (has(flags, VertexRawData::Normal))
			{
				std::memcpy(writePtr, &vertex.normal, sizeof(glm::vec3));
				writePtr += sizeof(glm::vec3);
			}
			if (has(flags, VertexRawData::Tangent))
			{
				std::memcpy(writePtr, &vertex.tangent, sizeof(glm::vec3));
				writePtr += sizeof(glm::vec3);
			}
			if (has(flags, VertexRawData::Bitangent))
			{
				std::memcpy(writePtr, &vertex.binormal, sizeof(glm::vec3));
				writePtr += sizeof(glm::vec3);
			}
			if (has(flags, VertexRawData::Color))
			{
				std::memcpy(writePtr, &vertex.color, sizeof(glm::vec4));
				writePtr += sizeof(glm::vec4);
			}
			if (has(flags, VertexRawData::TexCoord))
			{
				std::memcpy(writePtr, &vertex.texCoord, sizeof(glm::vec2));
				writePtr += sizeof(glm::vec2);
			}
		}

		return returnData;
	}

	std::vector<uint8_t> Geometry::GenerateRawIndex() const
	{
		std::vector<uint8_t> returnData(sizeof(u32) * m_indices.size());
		if (!m_indices.empty())
			std::memcpy(returnData.data(), m_indices.data(), returnData.size());
		return returnData;
	}
}
