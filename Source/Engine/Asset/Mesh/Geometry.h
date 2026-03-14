#pragma once

#include <Runtime/Data/Containers/ReadArray.h>

#include <vector>

namespace Horizon
{
	struct Vertex
	{
		Math::Vec3f position;
		Math::Vec3f normal;
		Math::Vec3f tangent;
		Math::Vec3f binormal;
		Math::Vec4f color;
		Math::Vec2f uv;
	};

	enum class VertexRawDataFlags
	{
		None = 0,
		Position = 1 << 0,
		Normal = 1 << 1,
		Tangent = 1 << 2,
		Binormal = 1 << 3,
		Color = 1 << 4,
		UV = 1 << 5,
		All = Position | Normal | Tangent | Binormal | Color | UV
	};

	class Geometry final
	{
	public:
		Geometry() = default;
		Geometry(u32 vertexLength, u32 indexLength);
		~Geometry();

		void AddPosition(Math::Vec3f position, u32 index);
		void AddNormal(Math::Vec3f normal, u32 index);
		void AddTangent(Math::Vec3f tangent, u32 index);
		void AddBinormal(Math::Vec3f binormal, u32 index);
		void AddColor(Math::Vec4f color, u32 index);
		void AddTexCoord(Math::Vec2f uv, u32 index);
		void AddIndex(u32 indexVal, u32 vecIndex);
		void AddIndex(const std::vector<u32>& indices);

		ReadArray<u8> GenerateRawVertex(VertexRawDataFlags flags) const;
		ReadArray<u8> GenerateRawIndex() const;

		inline const std::vector<Vertex>& GetVertexData() const { return m_vertexData; }
		inline std::vector<u32>& GetIndexData() { return m_indexData; }

		inline usize GetVertexCount() const { return m_vertexData.size(); }
		inline usize GetIndexCount() const { return m_indexData.size(); }

	private:
		std::vector<Vertex> m_vertexData;
		std::vector<u32> m_indexData;
	};
}