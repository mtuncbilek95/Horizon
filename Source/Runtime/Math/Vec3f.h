#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <cmath>
#include <string>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec3f : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Vec3f);
	public:
		static constexpr f32 SmallNumber = 1.e-8f;
		static constexpr f32 KindaSmallNumber = 1.e-4f;
		static constexpr f32 BigNumber = 3.4e+38f;
		static constexpr f32 Pi = 3.1415926535897932f;
		static constexpr f32 ThreshVectorsAreNear = 0.0004f;
		static constexpr f32 ThreshPointsAreSame = 0.00002f;
		static constexpr f32 ThreshPointsAreNear = 0.015f;
		static constexpr f32 ThreshNormalsAreParallel = 0.999845f;
		static constexpr f32 ThreshNormalsAreOrthogonal = 0.017455f;
		static constexpr f32 ThreshVectorNormalized = 0.01f;

		static const Vec3f ZeroVector;
		static const Vec3f OneVector;
		static const Vec3f UpVector;
		static const Vec3f DownVector;
		static const Vec3f ForwardVector;
		static const Vec3f BackwardVector;
		static const Vec3f RightVector;
		static const Vec3f LeftVector;
		static const Vec3f XAxisVector;
		static const Vec3f YAxisVector;
		static const Vec3f ZAxisVector;

		Vec3f()
			: m_x(0.f), m_y(0.f), m_z(0.f)
		{
		}

		explicit Vec3f(f32 value)
			: m_x(value), m_y(value), m_z(value)
		{
		}

		Vec3f(f32 x, f32 y, f32 z)
			: m_x(x), m_y(y), m_z(z)
		{
		}

		f32& X() { return m_x; }
		f32& Y() { return m_y; }
		f32& Z() { return m_z; }

		f32 X() const { return m_x; }
		f32 Y() const { return m_y; }
		f32 Z() const { return m_z; }

		void Set(f32 x, f32 y, f32 z)
		{
			m_x = x;
			m_y = y;
			m_z = z;
		}

		f32& operator[](i32 index)
		{
			if (index == 0)
				return m_x;

			if (index == 1)
				return m_y;

			return m_z;
		}

		f32 operator[](i32 index) const
		{
			if (index == 0)
				return m_x;

			if (index == 1)
				return m_y;

			return m_z;
		}

		f32 Component(i32 index) const { return (*this)[index]; }
		void SetComponent(i32 index, f32 value) { (*this)[index] = value; }

		Vec3f operator-() const { return Vec3f(-m_x, -m_y, -m_z); }
		Vec3f operator+(const Vec3f& other) const { return Vec3f(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z); }
		Vec3f operator-(const Vec3f& other) const { return Vec3f(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z); }
		Vec3f operator*(const Vec3f& other) const { return Vec3f(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z); }
		Vec3f operator/(const Vec3f& other) const { return Vec3f(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z); }
		Vec3f operator+(f32 scalar) const { return Vec3f(m_x + scalar, m_y + scalar, m_z + scalar); }
		Vec3f operator-(f32 scalar) const { return Vec3f(m_x - scalar, m_y - scalar, m_z - scalar); }
		Vec3f operator*(f32 scalar) const { return Vec3f(m_x * scalar, m_y * scalar, m_z * scalar); }

		Vec3f operator/(f32 scalar) const
		{
			const f32 inv = 1.f / scalar;
			return Vec3f(m_x * inv, m_y * inv, m_z * inv);
		}

		Vec3f& operator+=(const Vec3f& other)
		{
			m_x += other.m_x;
			m_y += other.m_y;
			m_z += other.m_z;
			return *this;
		}

		Vec3f& operator-=(const Vec3f& other)
		{
			m_x -= other.m_x;
			m_y -= other.m_y;
			m_z -= other.m_z;
			return *this;
		}

		Vec3f& operator*=(const Vec3f& other)
		{
			m_x *= other.m_x;
			m_y *= other.m_y;
			m_z *= other.m_z;
			return *this;
		}

		Vec3f& operator/=(const Vec3f& other)
		{
			m_x /= other.m_x;
			m_y /= other.m_y;
			m_z /= other.m_z;
			return *this;
		}

		Vec3f& operator*=(f32 scalar)
		{
			m_x *= scalar;
			m_y *= scalar;
			m_z *= scalar;
			return *this;
		}

		Vec3f& operator/=(f32 scalar)
		{
			const f32 inv = 1.f / scalar;
			m_x *= inv;
			m_y *= inv;
			m_z *= inv;
			return *this;
		}

		f32 operator|(const Vec3f& other) const
		{
			return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
		}

		Vec3f operator^(const Vec3f& other) const
		{
			return Vec3f(
				m_y * other.m_z - m_z * other.m_y,
				m_z * other.m_x - m_x * other.m_z,
				m_x * other.m_y - m_y * other.m_x);
		}

		f32 Dot(const Vec3f& other) const
		{
			return *this | other;
		}

		Vec3f Cross(const Vec3f& other) const
		{
			return *this ^ other;
		}

		b8 operator==(const Vec3f& other) const
		{
			return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
		}

		b8 operator!=(const Vec3f& other) const
		{
			return m_x != other.m_x || m_y != other.m_y || m_z != other.m_z;
		}

		b8 Equals(const Vec3f& other, f32 tolerance = KindaSmallNumber) const
		{
			return Abs(m_x - other.m_x) <= tolerance
				&& Abs(m_y - other.m_y) <= tolerance
				&& Abs(m_z - other.m_z) <= tolerance;
		}

		b8 AllComponentsEqual(f32 tolerance = KindaSmallNumber) const
		{
			return Abs(m_x - m_y) <= tolerance
				&& Abs(m_x - m_z) <= tolerance
				&& Abs(m_y - m_z) <= tolerance;
		}

		f32 SizeSquared() const
		{
			return m_x * m_x + m_y * m_y + m_z * m_z;
		}

		f32 Size() const
		{
			return std::sqrt(SizeSquared());
		}

		f32 SquaredLength() const
		{
			return SizeSquared();
		}

		f32 Length() const
		{
			return Size();
		}

		f32 SizeSquared2D() const
		{
			return m_x * m_x + m_y * m_y;
		}

		f32 Size2D() const
		{
			return std::sqrt(SizeSquared2D());
		}

		b8 IsNearlyZero(f32 tolerance = KindaSmallNumber) const
		{
			return Abs(m_x) <= tolerance && Abs(m_y) <= tolerance && Abs(m_z) <= tolerance;
		}

		b8 IsZero() const
		{
			return m_x == 0.f && m_y == 0.f && m_z == 0.f;
		}

		b8 IsUnit(f32 lengthSquaredTolerance = KindaSmallNumber) const
		{
			return Abs(1.f - SizeSquared()) < lengthSquaredTolerance;
		}

		b8 IsNormalized() const
		{
			return Abs(1.f - SizeSquared()) < ThreshVectorNormalized;
		}

		b8 Normalize(f32 tolerance = SmallNumber)
		{
			const f32 squareSum = SizeSquared();

			if (squareSum <= tolerance)
			{
				*this = ZeroVector;
				return false;
			}

			const f32 scale = 1.f / std::sqrt(squareSum);
			m_x *= scale;
			m_y *= scale;
			m_z *= scale;
			return true;
		}

		Vec3f GetSafeNormal(f32 tolerance = SmallNumber) const
		{
			const f32 squareSum = SizeSquared();

			if (squareSum == 1.f)
				return *this;

			if (squareSum < tolerance)
				return ZeroVector;

			const f32 scale = 1.f / std::sqrt(squareSum);
			return Vec3f(m_x * scale, m_y * scale, m_z * scale);
		}

		Vec3f GetSafeNormal2D(f32 tolerance = SmallNumber) const
		{
			const f32 squareSum = SizeSquared2D();

			if (squareSum == 1.f)
			{
				if (m_z == 0.f)
					return *this;

				return Vec3f(m_x, m_y, 0.f);
			}

			if (squareSum < tolerance)
				return ZeroVector;

			const f32 scale = 1.f / std::sqrt(squareSum);
			return Vec3f(m_x * scale, m_y * scale, 0.f);
		}

		Vec3f GetUnsafeNormal() const
		{
			const f32 scale = 1.f / Size();
			return Vec3f(m_x * scale, m_y * scale, m_z * scale);
		}

		void ToDirectionAndLength(Vec3f& outDirection, f32& outLength) const
		{
			outLength = Size();

			if (outLength > SmallNumber)
			{
				const f32 inv = 1.f / outLength;
				outDirection = Vec3f(m_x * inv, m_y * inv, m_z * inv);
				return;
			}

			outDirection = ZeroVector;
		}

		f32 GetMax() const { return Max(m_x, Max(m_y, m_z)); }
		f32 GetMin() const { return Min(m_x, Min(m_y, m_z)); }

		f32 GetAbsMax() const { return Max(Abs(m_x), Max(Abs(m_y), Abs(m_z))); }
		f32 GetAbsMin() const { return Min(Abs(m_x), Min(Abs(m_y), Abs(m_z))); }
		Vec3f GetAbs() const { return Vec3f(Abs(m_x), Abs(m_y), Abs(m_z)); }

		Vec3f ComponentMin(const Vec3f& other) const { return Vec3f(Min(m_x, other.m_x), Min(m_y, other.m_y), Min(m_z, other.m_z)); }
		Vec3f ComponentMax(const Vec3f& other) const { return Vec3f(Max(m_x, other.m_x), Max(m_y, other.m_y), Max(m_z, other.m_z)); }

		Vec3f GetSignVector() const { return Vec3f(Sign(m_x), Sign(m_y), Sign(m_z)); }

		Vec3f Reciprocal() const
		{
			Vec3f result;

			if (m_x != 0.f)
				result.m_x = 1.f / m_x;
			else
				result.m_x = BigNumber;

			if (m_y != 0.f)
				result.m_y = 1.f / m_y;
			else
				result.m_y = BigNumber;

			if (m_z != 0.f)
				result.m_z = 1.f / m_z;
			else
				result.m_z = BigNumber;

			return result;
		}

		b8 ContainsNaN() const { return !std::isfinite(m_x) || !std::isfinite(m_y) || !std::isfinite(m_z); }
		Vec3f ProjectOnTo(const Vec3f& other) const { return other * ((*this | other) / (other | other)); }
		Vec3f ProjectOnToNormal(const Vec3f& normal) const { return normal * (*this | normal); }
		Vec3f MirrorByVector(const Vec3f& mirrorNormal) const { return *this - mirrorNormal * (2.f * (*this | mirrorNormal)); }

		Vec3f RotateAngleAxis(f32 angleDegrees, const Vec3f& axis) const
		{
			const f32 radians = DegreesToRadians(angleDegrees);
			const f32 s = std::sin(radians);
			const f32 c = std::cos(radians);

			const f32 xx = axis.m_x * axis.m_x;
			const f32 yy = axis.m_y * axis.m_y;
			const f32 zz = axis.m_z * axis.m_z;

			const f32 xy = axis.m_x * axis.m_y;
			const f32 yz = axis.m_y * axis.m_z;
			const f32 zx = axis.m_z * axis.m_x;

			const f32 xs = axis.m_x * s;
			const f32 ys = axis.m_y * s;
			const f32 zs = axis.m_z * s;

			const f32 omc = 1.f - c;

			return Vec3f(
				(omc * xx + c) * m_x + (omc * xy - zs) * m_y + (omc * zx + ys) * m_z,
				(omc * xy + zs) * m_x + (omc * yy + c) * m_y + (omc * yz - xs) * m_z,
				(omc * zx - ys) * m_x + (omc * yz + xs) * m_y + (omc * zz + c) * m_z);
		}

		f32 HeadingAngle() const
		{
			Vec3f planeDir = Vec3f(m_x, m_y, 0.f).GetSafeNormal();
			f32 angle = std::acos(Clamp(planeDir.m_x, -1.f, 1.f));

			if (planeDir.m_y < 0.f)
				angle *= -1.f;

			return angle;
		}

		f32 CosineAngle2D(const Vec3f& other) const
		{
			const Vec3f a = Vec3f(m_x, m_y, 0.f).GetSafeNormal();
			const Vec3f b = Vec3f(other.m_x, other.m_y, 0.f).GetSafeNormal();
			return a | b;
		}

		void FindBestAxisVectors(Vec3f& outAxis1, Vec3f& outAxis2) const
		{
			const f32 nx = Abs(m_x);
			const f32 ny = Abs(m_y);
			const f32 nz = Abs(m_z);

			if (nz > nx && nz > ny)
				outAxis1 = XAxisVector;
			else if (ny > nz && ny > nx)
				outAxis1 = ZAxisVector;
			else
				outAxis1 = YAxisVector;

			const Vec3f temp = outAxis1 - *this * (outAxis1 | *this);
			outAxis1 = temp.GetSafeNormal();
			outAxis2 = outAxis1 ^ *this;
		}

		Vec3f GridSnap(f32 gridSize) const { return Vec3f(SnapToGrid(m_x, gridSize), SnapToGrid(m_y, gridSize), SnapToGrid(m_z, gridSize)); }
		Vec3f BoundToCube(f32 radius) const { return Vec3f(Clamp(m_x, -radius, radius), Clamp(m_y, -radius, radius), Clamp(m_z, -radius, radius)); }
		Vec3f BoundToBox(const Vec3f& min, const Vec3f& max) const { return Vec3f(Clamp(m_x, min.m_x, max.m_x), Clamp(m_y, min.m_y, max.m_y), Clamp(m_z, min.m_z, max.m_z)); }

		Vec3f GetClampedToSize(f32 min, f32 max) const
		{
			f32 vecSize = Size();
			Vec3f vecDir = ZeroVector;

			if (vecSize > SmallNumber)
				vecDir = *this / vecSize;

			vecSize = Clamp(vecSize, min, max);
			return vecDir * vecSize;
		}

		Vec3f GetClampedToMaxSize(f32 max) const
		{
			if (max < KindaSmallNumber)
				return ZeroVector;

			const f32 squareSum = SizeSquared();

			if (squareSum > max * max)
			{
				const f32 scale = max / std::sqrt(squareSum);
				return Vec3f(m_x * scale, m_y * scale, m_z * scale);
			}

			return *this;
		}

		Vec3f GetClampedToSize2D(f32 min, f32 max) const
		{
			f32 vecSize2D = Size2D();
			Vec3f vecDir = ZeroVector;

			if (vecSize2D > SmallNumber)
				vecDir = Vec3f(m_x / vecSize2D, m_y / vecSize2D, 0.f);

			vecSize2D = Clamp(vecSize2D, min, max);
			return Vec3f(vecDir.m_x * vecSize2D, vecDir.m_y * vecSize2D, m_z);
		}

		Vec3f GetClampedToMaxSize2D(f32 max) const
		{
			if (max < KindaSmallNumber)
				return Vec3f(0.f, 0.f, m_z);

			const f32 squareSum2D = SizeSquared2D();

			if (squareSum2D > max * max)
			{
				const f32 scale = max / std::sqrt(squareSum2D);
				return Vec3f(m_x * scale, m_y * scale, m_z);
			}

			return *this;
		}

		void AddBounded(const Vec3f& value, f32 radius) { *this = (*this + value).BoundToCube(radius); }

		std::string ToString() const { return "X=" + std::to_string(m_x) + " Y=" + std::to_string(m_y) + " Z=" + std::to_string(m_z); }

		static f32 DotProduct(const Vec3f& a, const Vec3f& b) { return a | b; }
		static Vec3f CrossProduct(const Vec3f& a, const Vec3f& b) { return a ^ b; }

		static f32 Triple(const Vec3f& x, const Vec3f& y, const Vec3f& z)
		{
			return x.m_x * (y.m_y * z.m_z - y.m_z * z.m_y)
				+ x.m_y * (y.m_z * z.m_x - y.m_x * z.m_z)
				+ x.m_z * (y.m_x * z.m_y - y.m_y * z.m_x);
		}

		static f32 DistSquared(const Vec3f& a, const Vec3f& b) { return (b - a).SizeSquared(); }
		static f32 Dist(const Vec3f& a, const Vec3f& b) { return std::sqrt(DistSquared(a, b)); }
		static f32 Distance(const Vec3f& a, const Vec3f& b) { return Dist(a, b); }
		static f32 DistSquared2D(const Vec3f& a, const Vec3f& b) { return (b - a).SizeSquared2D(); }
		static f32 Dist2D(const Vec3f& a, const Vec3f& b) { return std::sqrt(DistSquared2D(a, b)); }
		static f32 DistSquaredXY(const Vec3f& a, const Vec3f& b) { return DistSquared2D(a, b); }
		static f32 DistXY(const Vec3f& a, const Vec3f& b) { return Dist2D(a, b); }
		static Vec3f Min(const Vec3f& a, const Vec3f& b) { return a.ComponentMin(b); }
		static Vec3f Max(const Vec3f& a, const Vec3f& b) { return a.ComponentMax(b); }
		static Vec3f Lerp(const Vec3f& a, const Vec3f& b, f32 alpha) { return a + (b - a) * alpha; }
		static b8 Parallel(const Vec3f& normal1, const Vec3f& normal2, f32 parallelCosineThreshold = ThreshNormalsAreParallel) { return Abs(normal1 | normal2) >= parallelCosineThreshold; }
		static b8 Coincident(const Vec3f& normal1, const Vec3f& normal2, f32 parallelCosineThreshold = ThreshNormalsAreParallel) { return (normal1 | normal2) >= parallelCosineThreshold; }
		static b8 Orthogonal(const Vec3f& normal1, const Vec3f& normal2, f32 orthogonalCosineThreshold = ThreshNormalsAreOrthogonal) { return Abs(normal1 | normal2) <= orthogonalCosineThreshold; }

		static b8 Coplanar(const Vec3f& base1, const Vec3f& normal1, const Vec3f& base2, const Vec3f& normal2, f32 parallelCosineThreshold = ThreshNormalsAreParallel)
		{
			if (!Parallel(normal1, normal2, parallelCosineThreshold))
				return false;

			if (Abs(PointPlaneDist(base2, base1, normal1)) > ThreshPointsAreSame)
				return false;

			return true;
		}

		static f32 PointPlaneDist(const Vec3f& point, const Vec3f& planeBase, const Vec3f& planeNormal) { return (point - planeBase) | planeNormal; }
		static Vec3f PointPlaneProject(const Vec3f& point, const Vec3f& planeBase, const Vec3f& planeNormal) { return point - planeNormal * PointPlaneDist(point, planeBase, planeNormal); }
		static Vec3f VectorPlaneProject(const Vec3f& value, const Vec3f& planeNormal) { return value - value.ProjectOnToNormal(planeNormal); }
		static b8 PointsAreSame(const Vec3f& p, const Vec3f& q) { return (q - p).GetAbsMax() < ThreshPointsAreSame; }
		static b8 PointsAreNear(const Vec3f& p, const Vec3f& q, f32 distance) { return (q - p).GetAbsMax() < distance; }
		static f32 DegreesToRadians(f32 degrees) { return degrees * (Pi / 180.f); }
		static f32 RadiansToDegrees(f32 radians) { return radians * (180.f / Pi); }

	private:
		static f32 Abs(f32 value) 
		{
			if (value < 0.f)
				return -value;

			return value;
		}

		static f32 Min(f32 a, f32 b)
		{
			if (a < b)
				return a;

			return b;
		}

		static f32 Max(f32 a, f32 b)
		{
			if (a > b)
				return a;

			return b;
		}

		static f32 Clamp(f32 value, f32 min, f32 max)
		{
			if (value < min)
				return min;

			if (value > max)
				return max;

			return value;
		}

		static f32 Sign(f32 value)
		{
			if (value > 0.f)
				return 1.f;

			if (value < 0.f)
				return -1.f;

			return 0.f;
		}

		static f32 SnapToGrid(f32 value, f32 gridSize)
		{
			if (gridSize == 0.f)
				return value;

			return std::floor((value + 0.5f * gridSize) / gridSize) * gridSize;
		}

	private:
		HFIELD();
		f32 m_x;

		HFIELD();
		f32 m_y;

		HFIELD();
		f32 m_z;
	};

	inline Vec3f operator*(f32 scalar, const Vec3f& value)
	{
		return value * scalar;
	}

	inline const Vec3f Vec3f::ZeroVector = Vec3f(0.f, 0.f, 0.f);
	inline const Vec3f Vec3f::OneVector = Vec3f(1.f, 1.f, 1.f);
	inline const Vec3f Vec3f::UpVector = Vec3f(0.f, 0.f, 1.f);
	inline const Vec3f Vec3f::DownVector = Vec3f(0.f, 0.f, -1.f);
	inline const Vec3f Vec3f::ForwardVector = Vec3f(1.f, 0.f, 0.f);
	inline const Vec3f Vec3f::BackwardVector = Vec3f(-1.f, 0.f, 0.f);
	inline const Vec3f Vec3f::RightVector = Vec3f(0.f, 1.f, 0.f);
	inline const Vec3f Vec3f::LeftVector = Vec3f(0.f, -1.f, 0.f);
	inline const Vec3f Vec3f::XAxisVector = Vec3f(1.f, 0.f, 0.f);
	inline const Vec3f Vec3f::YAxisVector = Vec3f(0.f, 1.f, 0.f);
	inline const Vec3f Vec3f::ZAxisVector = Vec3f(0.f, 0.f, 1.f);
}
