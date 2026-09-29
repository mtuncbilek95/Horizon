#include "DebugDraw.h"

namespace Horizon::Engine
{
	void DebugDraw::Line(const Math::Vec3f& from, const Math::Vec3f& to, u32 color)
	{
		m_vertices.PushBack({ from, color });
		m_vertices.PushBack({ to, color });
	}

	void DebugDraw::Box(const Math::Vec3f& min, const Math::Vec3f& max, u32 color)
	{
		const Math::Vec3f c[8] =
		{
			{ min.X(), min.Y(), min.Z() }, { max.X(), min.Y(), min.Z() },
			{ max.X(), max.Y(), min.Z() }, { min.X(), max.Y(), min.Z() },
			{ min.X(), min.Y(), max.Z() }, { max.X(), min.Y(), max.Z() },
			{ max.X(), max.Y(), max.Z() }, { min.X(), max.Y(), max.Z() }
		};

		static constexpr u8 edges[12][2] =
		{
			{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
			{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
			{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
		};

		for (const auto& edge : edges)
			Line(c[edge[0]], c[edge[1]], color);
	}

	void DebugDraw::Axis(const Math::Vec3f& origin, f32 size)
	{
		Line(origin, origin + Math::Vec3f(size, 0.f, 0.f), Rgba(255, 0, 0));
		Line(origin, origin + Math::Vec3f(0.f, size, 0.f), Rgba(0, 255, 0));
		Line(origin, origin + Math::Vec3f(0.f, 0.f, size), Rgba(0, 0, 255));
	}
}