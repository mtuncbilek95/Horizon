#include "BoundingBox.h"

#include <Runtime/Math/Scalar.h>

namespace Horizon::Math
{
	BoundingBox BoundingBox::Empty()
	{
		return BoundingBox(Vec3f(BigNumber), Vec3f(-BigNumber));
	}

	BoundingBox BoundingBox::FromCenterExtents(const Vec3f& center, const Vec3f& extents)
	{
		return BoundingBox(center - extents, center + extents);
	}

	BoundingBox BoundingBox::FromPoints(const Vec3f* pPoints, u64 count)
	{
		BoundingBox box = Empty();

		for (u64 i = 0; i < count; ++i)
			box.Expand(pPoints[i]);

		return box;
	}

	BoundingBox::BoundingBox() : m_min(BigNumber), m_max(-BigNumber)
	{
	}

	BoundingBox::BoundingBox(const Vec3f& min, const Vec3f& max) : m_min(min), m_max(max)
	{
	}

	Vec3f BoundingBox::Center() const
	{
		return (m_min + m_max) * 0.5f;
	}

	Vec3f BoundingBox::Extents() const
	{
		return (m_max - m_min) * 0.5f;
	}

	Vec3f BoundingBox::Size() const
	{
		return m_max - m_min;
	}

	f32 BoundingBox::Volume() const
	{
		if (!IsValid())
			return 0.f;

		const Vec3f size = Size();
		return size.X() * size.Y() * size.Z();
	}

	b8 BoundingBox::IsValid() const
	{
		return m_min.X() <= m_max.X() && m_min.Y() <= m_max.Y() && m_min.Z() <= m_max.Z();
	}

	void BoundingBox::Expand(const Vec3f& point)
	{
		m_min.Set(Math::Min(m_min.X(), point.X()), Math::Min(m_min.Y(), point.Y()), Math::Min(m_min.Z(), point.Z()));
		m_max.Set(Math::Max(m_max.X(), point.X()), Math::Max(m_max.Y(), point.Y()), Math::Max(m_max.Z(), point.Z()));
	}

	void BoundingBox::Expand(const BoundingBox& other)
	{
		if (!other.IsValid())
			return;

		Expand(other.m_min);
		Expand(other.m_max);
	}

	void BoundingBox::Inflate(f32 amount)
	{
		m_min -= amount;
		m_max += amount;
	}

	b8 BoundingBox::Contains(const Vec3f& point) const
	{
		return point.X() >= m_min.X() && point.X() <= m_max.X()
			&& point.Y() >= m_min.Y() && point.Y() <= m_max.Y()
			&& point.Z() >= m_min.Z() && point.Z() <= m_max.Z();
	}

	b8 BoundingBox::Contains(const BoundingBox& other) const
	{
		return Contains(other.m_min) && Contains(other.m_max);
	}

	b8 BoundingBox::Intersects(const BoundingBox& other) const
	{
		if (!IsValid() || !other.IsValid())
			return false;

		return m_min.X() <= other.m_max.X() && m_max.X() >= other.m_min.X()
			&& m_min.Y() <= other.m_max.Y() && m_max.Y() >= other.m_min.Y()
			&& m_min.Z() <= other.m_max.Z() && m_max.Z() >= other.m_min.Z();
	}

	BoundingBox BoundingBox::Transformed(const Mat4f& transform) const
	{
		if (!IsValid())
			return *this;

		BoundingBox result = Empty();

		for (i32 i = 0; i < 8; ++i)
		{
			const Vec3f corner(
				(i & 1) ? m_max.X() : m_min.X(),
				(i & 2) ? m_max.Y() : m_min.Y(),
				(i & 4) ? m_max.Z() : m_min.Z());

			result.Expand(transform.TransformPoint(corner));
		}

		return result;
	}

	b8 BoundingBox::operator==(const BoundingBox& other) const
	{
		return m_min == other.m_min && m_max == other.m_max;
	}

	b8 BoundingBox::operator!=(const BoundingBox& other) const
	{
		return !(*this == other);
	}
}