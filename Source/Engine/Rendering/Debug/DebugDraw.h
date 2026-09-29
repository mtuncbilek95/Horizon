#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct DebugVertex
	{
		Math::Vec3f position;
		u32 color;
	};

	class H_EXPORT DebugDraw final
	{
	public:
		static constexpr u32 Rgba(u8 r, u8 g, u8 b, u8 a = 255)
		{
			return u32(r) | (u32(g) << 8) | (u32(b) << 16) | (u32(a) << 24);
		}

		void Line(const Math::Vec3f& from, const Math::Vec3f& to, u32 color);
		void Box(const Math::Vec3f& min, const Math::Vec3f& max, u32 color);
		void Axis(const Math::Vec3f& origin, f32 size);

		void Clear() { m_vertices.Clear(); }

		const List<DebugVertex>& GetVertices() const { return m_vertices; }
		usize GetLineCount() const { return m_vertices.GetCount() / 2; }

	private:
		List<DebugVertex> m_vertices;
	};
}