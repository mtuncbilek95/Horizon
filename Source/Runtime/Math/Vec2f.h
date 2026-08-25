#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <cmath>
#include <string>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec2f : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Vec2f);
	public:
		static constexpr f32 SmallNumber = 1.e-8f;
		static constexpr f32 KindaSmallNumber = 1.e-4f;
		static constexpr f32 BigNumber = 3.4e+38f;
		static constexpr f32 Pi = 3.1415926535897932f;
		static constexpr f32 InvSqrt2 = 0.7071067811865475f;
		static constexpr f32 ThreshVectorsAreNear = 0.0004f;
		static constexpr f32 ThreshPointsAreSame = 0.00002f;
		static constexpr f32 ThreshNormalsAreParallel = 0.999845f;
		static constexpr f32 ThreshNormalsAreOrthogonal = 0.017455f;
		static constexpr f32 ThreshVectorNormalized = 0.01f;

		static const Vec2f ZeroVector;
		static const Vec2f OneVector;
		static const Vec2f UnitVector;
		static const Vec2f Unit45Deg;
		static const Vec2f UpVector;
		static const Vec2f DownVector;
		static const Vec2f RightVector;
		static const Vec2f LeftVector;
		static const Vec2f XAxisVector;
		static const Vec2f YAxisVector;

		Vec2f() : m_x(0.f), m_y(0.f)
		{
		}

		explicit Vec2f(f32 value) : m_x(value), m_y(value)
		{
		}

		Vec2f(f32 x, f32 y) : m_x(x), m_y(y)
		{
		}

		f32& X() { return m_x; }
		f32& Y() { return m_y; }

		f32 X() const { return m_x; }
		f32 Y() const { return m_y; }

		void Set(f32 x, f32 y)
		{
			m_x = x;
			m_y = y;
		}

		f32& operator[](i32 index)
		{
			if (index == 0)
				return m_x;

			return m_y;
		}

		f32 operator[](i32 index) const
		{
			if (index == 0)
				return m_x;

			return m_y;
		}

		f32 Component(i32 index) const { return (*this)[index]; }
		void SetComponent(i32 index, f32 value) { (*this)[index] = value; }

		Vec2f operator-() const { return Vec2f(-m_x, -m_y); }
		Vec2f operator+(const Vec2f& other) const { return Vec2f(m_x + other.m_x, m_y + other.m_y); }
		Vec2f operator-(const Vec2f& other) const { return Vec2f(m_x - other.m_x, m_y - other.m_y); }
		Vec2f operator*(const Vec2f& other) const { return Vec2f(m_x * other.m_x, m_y * other.m_y); }
		Vec2f operator/(const Vec2f& other) const { return Vec2f(m_x / other.m_x, m_y / other.m_y); }
		Vec2f operator+(f32 scalar) const { return Vec2f(m_x + scalar, m_y + scalar); }
		Vec2f operator-(f32 scalar) const { return Vec2f(m_x - scalar, m_y - scalar); }
		Vec2f operator*(f32 scalar) const { return Vec2f(m_x * scalar, m_y * scalar); }

		Vec2f operator/(f32 scalar) const
		{
			const f32 inv = 1.f / scalar;
			return Vec2f(m_x * inv, m_y * inv);
		}

		Vec2f& operator+=(const Vec2f& other)
		{
			m_x += other.m_x;
			m_y += other.m_y;
			return *this;
		}

		Vec2f& operator-=(const Vec2f& other)
		{
			m_x -= other.m_x;
			m_y -= other.m_y;
			return *this;
		}

		Vec2f& operator*=(const Vec2f& other)
		{
			m_x *= other.m_x;
			m_y *= other.m_y;
			return *this;
		}

		Vec2f& operator/=(const Vec2f& other)
		{
			m_x /= other.m_x;
			m_y /= other.m_y;
			return *this;
		}

		Vec2f& operator*=(f32 scalar)
		{
			m_x *= scalar;
			m_y *= scalar;
			return *this;
		}

		Vec2f& operator/=(f32 scalar)
		{
			const f32 inv = 1.f / scalar;
			m_x *= inv;
			m_y *= inv;
			return *this;
		}

		f32 operator|(const Vec2f& other) const { return m_x * other.m_x + m_y * other.m_y; }
		f32 operator^(const Vec2f& other) const { return m_x * other.m_y - m_y * other.m_x; }

		f32 Dot(const Vec2f& other) const { return *this | other; }
		f32 Cross(const Vec2f& other) const { return *this ^ other; }

		b8 operator==(const Vec2f& other) const { return m_x == other.m_x && m_y == other.m_y; }
		b8 operator!=(const Vec2f& other) const { return m_x != other.m_x || m_y != other.m_y; }
		b8 operator<(const Vec2f& other) const { return m_x < other.m_x && m_y < other.m_y; }
		b8 operator>(const Vec2f& other) const { return m_x > other.m_x && m_y > other.m_y; }
		b8 operator<=(const Vec2f& other) const { return m_x <= other.m_x && m_y <= other.m_y; }
		b8 operator>=(const Vec2f& other) const { return m_x >= other.m_x && m_y >= other.m_y; }

		b8 Equals(const Vec2f& other, f32 tolerance = KindaSmallNumber) const
		{
			return Abs(m_x - other.m_x) <= tolerance
				&& Abs(m_y - other.m_y) <= tolerance;
		}

		b8 AllComponentsEqual(f32 tolerance = KindaSmallNumber) const { return Abs(m_x - m_y) <= tolerance; }

		f32 SizeSquared() const { return m_x * m_x + m_y * m_y; }
		f32 Size() const { return std::sqrt(SizeSquared()); }
		f32 SquaredLength() const { return SizeSquared(); }
		f32 Length() const { return Size(); }

		b8 IsNearlyZero(f32 tolerance = KindaSmallNumber) const { return Abs(m_x) <= tolerance && Abs(m_y) <= tolerance; }
		b8 IsZero() const { return m_x == 0.f && m_y == 0.f; }
		b8 IsUnit(f32 lengthSquaredTolerance = KindaSmallNumber) const { return Abs(1.f - SizeSquared()) < lengthSquaredTolerance; }
		b8 IsNormalized() const { return Abs(1.f - SizeSquared()) < ThreshVectorNormalized; }

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
			return true;
		}

		Vec2f GetSafeNormal(f32 tolerance = SmallNumber) const
		{
			const f32 squareSum = SizeSquared();

			if (squareSum == 1.f)
				return *this;

			if (squareSum < tolerance)
				return ZeroVector;

			const f32 scale = 1.f / std::sqrt(squareSum);
			return Vec2f(m_x * scale, m_y * scale);
		}

		Vec2f GetUnsafeNormal() const
		{
			const f32 scale = 1.f / Size();
			return Vec2f(m_x * scale, m_y * scale);
		}

		void ToDirectionAndLength(Vec2f& outDirection, f32& outLength) const
		{
			outLength = Size();

			if (outLength > SmallNumber)
			{
				const f32 inv = 1.f / outLength;
				outDirection = Vec2f(m_x * inv, m_y * inv);
				return;
			}

			outDirection = ZeroVector;
		}

		f32 GetMax() const { return Max(m_x, m_y); }
		f32 GetMin() const { return Min(m_x, m_y); }

		f32 GetAbsMax() const { return Max(Abs(m_x), Abs(m_y)); }
		f32 GetAbsMin() const { return Min(Abs(m_x), Abs(m_y)); }
		Vec2f GetAbs() const { return Vec2f(Abs(m_x), Abs(m_y)); }

		Vec2f ComponentMin(const Vec2f& other) const { return Vec2f(Min(m_x, other.m_x), Min(m_y, other.m_y)); }
		Vec2f ComponentMax(const Vec2f& other) const { return Vec2f(Max(m_x, other.m_x), Max(m_y, other.m_y)); }

		Vec2f GetSignVector() const { return Vec2f(Sign(m_x), Sign(m_y)); }

		Vec2f Reciprocal() const
		{
			Vec2f result;

			if (m_x != 0.f)
				result.m_x = 1.f / m_x;
			else
				result.m_x = BigNumber;

			if (m_y != 0.f)
				result.m_y = 1.f / m_y;
			else
				result.m_y = BigNumber;

			return result;
		}

		b8 ContainsNaN() const { return !std::isfinite(m_x) || !std::isfinite(m_y); }

		Vec2f ProjectOnTo(const Vec2f& other) const { return other * ((*this | other) / (other | other)); }
		Vec2f ProjectOnToNormal(const Vec2f& normal) const { return normal * (*this | normal); }
		Vec2f MirrorByVector(const Vec2f& mirrorNormal) const { return *this - mirrorNormal * (2.f * (*this | mirrorNormal)); }

		Vec2f GetPerpendicular() const { return Vec2f(-m_y, m_x); }

		Vec2f GetRotated(f32 angleDegrees) const
		{
			const f32 radians = DegreesToRadians(angleDegrees);
			const f32 s = std::sin(radians);
			const f32 c = std::cos(radians);
			return Vec2f(c * m_x - s * m_y, s * m_x + c * m_y);
		}

		f32 HeadingAngle() const
		{
			const Vec2f direction = GetSafeNormal();
			f32 angle = std::acos(Clamp(direction.m_x, -1.f, 1.f));

			if (direction.m_y < 0.f)
				angle *= -1.f;

			return angle;
		}

		f32 CosineAngle(const Vec2f& other) const { return GetSafeNormal() | other.GetSafeNormal(); }

		Vec2f GridSnap(f32 gridSize) const { return Vec2f(SnapToGrid(m_x, gridSize), SnapToGrid(m_y, gridSize)); }
		Vec2f ClampAxes(f32 minAxisValue, f32 maxAxisValue) const { return Vec2f(Clamp(m_x, minAxisValue, maxAxisValue), Clamp(m_y, minAxisValue, maxAxisValue)); }
		Vec2f BoundToBox(const Vec2f& min, const Vec2f& max) const { return Vec2f(Clamp(m_x, min.m_x, max.m_x), Clamp(m_y, min.m_y, max.m_y)); }

		Vec2f RoundToVector() const { return Vec2f(std::round(m_x), std::round(m_y)); }
		Vec2f FloorToVector() const { return Vec2f(std::floor(m_x), std::floor(m_y)); }
		Vec2f CeilToVector() const { return Vec2f(std::ceil(m_x), std::ceil(m_y)); }

		Vec2f GetClampedToSize(f32 min, f32 max) const
		{
			f32 vecSize = Size();
			Vec2f vecDir = ZeroVector;

			if (vecSize > SmallNumber)
				vecDir = *this / vecSize;

			vecSize = Clamp(vecSize, min, max);
			return vecDir * vecSize;
		}

		Vec2f GetClampedToMaxSize(f32 max) const
		{
			if (max < KindaSmallNumber)
				return ZeroVector;

			const f32 squareSum = SizeSquared();

			if (squareSum > max * max)
			{
				const f32 scale = max / std::sqrt(squareSum);
				return Vec2f(m_x * scale, m_y * scale);
			}

			return *this;
		}

		std::string ToString() const { return "X=" + std::to_string(m_x) + " Y=" + std::to_string(m_y); }

		static f32 DotProduct(const Vec2f& a, const Vec2f& b) { return a | b; }
		static f32 CrossProduct(const Vec2f& a, const Vec2f& b) { return a ^ b; }

		static f32 DistSquared(const Vec2f& a, const Vec2f& b) { return (b - a).SizeSquared(); }
		static f32 Dist(const Vec2f& a, const Vec2f& b) { return std::sqrt(DistSquared(a, b)); }
		static f32 Distance(const Vec2f& a, const Vec2f& b) { return Dist(a, b); }

		static Vec2f Min(const Vec2f& a, const Vec2f& b) { return a.ComponentMin(b); }
		static Vec2f Max(const Vec2f& a, const Vec2f& b) { return a.ComponentMax(b); }
		static Vec2f Lerp(const Vec2f& a, const Vec2f& b, f32 alpha) { return a + (b - a) * alpha; }

		static b8 Parallel(const Vec2f& normal1, const Vec2f& normal2, f32 parallelCosineThreshold = ThreshNormalsAreParallel) { return Abs(normal1 | normal2) >= parallelCosineThreshold; }
		static b8 Coincident(const Vec2f& normal1, const Vec2f& normal2, f32 parallelCosineThreshold = ThreshNormalsAreParallel) { return (normal1 | normal2) >= parallelCosineThreshold; }
		static b8 Orthogonal(const Vec2f& normal1, const Vec2f& normal2, f32 orthogonalCosineThreshold = ThreshNormalsAreOrthogonal) { return Abs(normal1 | normal2) <= orthogonalCosineThreshold; }

		static b8 PointsAreSame(const Vec2f& p, const Vec2f& q) { return (q - p).GetAbsMax() < ThreshPointsAreSame; }
		static b8 PointsAreNear(const Vec2f& p, const Vec2f& q, f32 distance) { return (q - p).GetAbsMax() < distance; }

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
	};

	inline Vec2f operator*(f32 scalar, const Vec2f& value)
	{
		return value * scalar;
	}

	inline const Vec2f Vec2f::ZeroVector = Vec2f(0.f, 0.f);
	inline const Vec2f Vec2f::OneVector = Vec2f(1.f, 1.f);
	inline const Vec2f Vec2f::UnitVector = Vec2f(1.f, 1.f);
	inline const Vec2f Vec2f::Unit45Deg = Vec2f(InvSqrt2, InvSqrt2);
	inline const Vec2f Vec2f::UpVector = Vec2f(0.f, 1.f);
	inline const Vec2f Vec2f::DownVector = Vec2f(0.f, -1.f);
	inline const Vec2f Vec2f::RightVector = Vec2f(1.f, 0.f);
	inline const Vec2f Vec2f::LeftVector = Vec2f(-1.f, 0.f);
	inline const Vec2f Vec2f::XAxisVector = Vec2f(1.f, 0.f);
	inline const Vec2f Vec2f::YAxisVector = Vec2f(0.f, 1.f);
}
