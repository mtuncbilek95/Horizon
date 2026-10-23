#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class RUNTIME_API BoundingBox final
	{
		HORIZON_DATA_REFLECT(BoundingBox);
	public:
		static BoundingBox Empty();
		static BoundingBox FromCenterExtents(const Vec3f& center, const Vec3f& extents);
		static BoundingBox FromPoints(const Vec3f* pPoints, u64 count);

	public:
		BoundingBox();
		BoundingBox(const Vec3f& min, const Vec3f& max);
		~BoundingBox() = default;

		BoundingBox(const BoundingBox&) = default;
		BoundingBox& operator=(const BoundingBox&) = default;

		BoundingBox(BoundingBox&&) = default;
		BoundingBox& operator=(BoundingBox&&) = default;

		const Vec3f& Min() const { return m_min; }
		Vec3f& Min() { return m_min; }

		const Vec3f& Max() const { return m_max; }
		Vec3f& Max() { return m_max; }

		Vec3f Center() const;
		Vec3f Extents() const;
		Vec3f Size() const;
		f32 Volume() const;
		b8 IsValid() const;

		void Expand(const Vec3f& point);
		void Expand(const BoundingBox& other);
		void Inflate(f32 amount);

		b8 Contains(const Vec3f& point) const;
		b8 Contains(const BoundingBox& other) const;
		b8 Intersects(const BoundingBox& other) const;

		BoundingBox Transformed(const Mat4f& transform) const;

		b8 operator==(const BoundingBox& other) const;
		b8 operator!=(const BoundingBox& other) const;

	private:
		HFIELD();
		Vec3f m_min;

		HFIELD();
		Vec3f m_max;
	};
}