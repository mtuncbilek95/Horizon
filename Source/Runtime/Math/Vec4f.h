#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <Runtime/Math/Vec3f.h>

#include <cmath>
#include <string>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec4f : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Vec4f);
	public:
		static constexpr f32 SmallNumber = 1.e-8f;
		static constexpr f32 KindaSmallNumber = 1.e-4f;
		static constexpr f32 BigNumber = 3.4e+38f;
		static constexpr f32 Pi = 3.1415926535897932f;
		static constexpr f32 ThreshVectorNormalized = 0.01f;

		static const Vec4f Zero;
		static const Vec4f One;
		static const Vec4f XAxisVector;
		static const Vec4f YAxisVector;
		static const Vec4f ZAxisVector;
		static const Vec4f WAxisVector;

		Vec4f() : m_x(0.f), m_y(0.f), m_z(0.f), m_w(0.f)
		{
		}

		explicit Vec4f(f32 value) : m_x(value), m_y(value), m_z(value), m_w(value)
		{
		}

		Vec4f(f32 x, f32 y, f32 z, f32 w = 1.f) : m_x(x), m_y(y), m_z(z), m_w(w)
		{
		}

		explicit Vec4f(const Vec3f& value, f32 w = 1.f) : m_x(value.X()), m_y(value.Y()), m_z(value.Z()), m_w(w)
		{
		}

		f32& X() { return m_x; }
		f32& Y() { return m_y; }
		f32& Z() { return m_z; }
		f32& W() { return m_w; }

		f32 X() const { return m_x; }
		f32 Y() const { return m_y; }
		f32 Z() const { return m_z; }
		f32 W() const { return m_w; }

		void Set(f32 x, f32 y, f32 z, f32 w)
		{
			m_x = x;
			m_y = y;
			m_z = z;
			m_w = w;
		}

		f32& operator[](i32 index)
		{
			if (index == 0)
				return m_x;

			if (index == 1)
				return m_y;

			if (index == 2)
				return m_z;

			return m_w;
		}

		f32 operator[](i32 index) const
		{
			if (index == 0)
				return m_x;

			if (index == 1)
				return m_y;

			if (index == 2)
				return m_z;

			return m_w;
		}

		f32 Component(i32 index) const { return (*this)[index]; }
		void SetComponent(i32 index, f32 value) { (*this)[index] = value; }

		Vec3f ToVec3f() const { return Vec3f(m_x, m_y, m_z); }

		Vec4f operator-() const { return Vec4f(-m_x, -m_y, -m_z, -m_w); }
		Vec4f operator+(const Vec4f& other) const { return Vec4f(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w); }
		Vec4f operator-(const Vec4f& other) const { return Vec4f(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w); }
		Vec4f operator*(const Vec4f& other) const { return Vec4f(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z, m_w * other.m_w); }
		Vec4f operator/(const Vec4f& other) const { return Vec4f(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z, m_w / other.m_w); }
		Vec4f operator+(f32 scalar) const { return Vec4f(m_x + scalar, m_y + scalar, m_z + scalar, m_w + scalar); }
		Vec4f operator-(f32 scalar) const { return Vec4f(m_x - scalar, m_y - scalar, m_z - scalar, m_w - scalar); }
		Vec4f operator*(f32 scalar) const { return Vec4f(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar); }

		Vec4f operator/(f32 scalar) const
		{
			const f32 inv = 1.f / scalar;
			return Vec4f(m_x * inv, m_y * inv, m_z * inv, m_w * inv);
		}

		Vec4f& operator+=(const Vec4f& other)
		{
			m_x += other.m_x;
			m_y += other.m_y;
			m_z += other.m_z;
			m_w += other.m_w;
			return *this;
		}

		Vec4f& operator-=(const Vec4f& other)
		{
			m_x -= other.m_x;
			m_y -= other.m_y;
			m_z -= other.m_z;
			m_w -= other.m_w;
			return *this;
		}

		Vec4f& operator*=(const Vec4f& other)
		{
			m_x *= other.m_x;
			m_y *= other.m_y;
			m_z *= other.m_z;
			m_w *= other.m_w;
			return *this;
		}

		Vec4f& operator/=(const Vec4f& other)
		{
			m_x /= other.m_x;
			m_y /= other.m_y;
			m_z /= other.m_z;
			m_w /= other.m_w;
			return *this;
		}

		Vec4f& operator*=(f32 scalar)
		{
			m_x *= scalar;
			m_y *= scalar;
			m_z *= scalar;
			m_w *= scalar;
			return *this;
		}

		Vec4f& operator/=(f32 scalar)
		{
			const f32 inv = 1.f / scalar;
			m_x *= inv;
			m_y *= inv;
			m_z *= inv;
			m_w *= inv;
			return *this;
		}

		f32 operator|(const Vec4f& other) const { return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z + m_w * other.m_w; }

		Vec4f operator^(const Vec4f& other) const
		{
			return Vec4f(
				m_y * other.m_z - m_z * other.m_y,
				m_z * other.m_x - m_x * other.m_z,
				m_x * other.m_y - m_y * other.m_x,
				0.f);
		}

		f32 Dot3(const Vec4f& other) const { return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z; }
		f32 Dot4(const Vec4f& other) const { return *this | other; }
		Vec4f Cross3(const Vec4f& other) const { return *this ^ other; }

		b8 operator==(const Vec4f& other) const { return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z && m_w == other.m_w; }
		b8 operator!=(const Vec4f& other) const { return m_x != other.m_x || m_y != other.m_y || m_z != other.m_z || m_w != other.m_w; }

		b8 Equals(const Vec4f& other, f32 tolerance = KindaSmallNumber) const
		{
			return Abs(m_x - other.m_x) <= tolerance
				&& Abs(m_y - other.m_y) <= tolerance
				&& Abs(m_z - other.m_z) <= tolerance
				&& Abs(m_w - other.m_w) <= tolerance;
		}

		b8 AllComponentsEqual(f32 tolerance = KindaSmallNumber) const
		{
			return Abs(m_x - m_y) <= tolerance
				&& Abs(m_x - m_z) <= tolerance
				&& Abs(m_x - m_w) <= tolerance;
		}

		f32 SizeSquared3() const { return m_x * m_x + m_y * m_y + m_z * m_z; }
		f32 Size3() const { return std::sqrt(SizeSquared3()); }
		f32 SquaredLength3() const { return SizeSquared3(); }
		f32 Length3() const { return Size3(); }

		f32 SizeSquared() const { return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w; }
		f32 Size() const { return std::sqrt(SizeSquared()); }
		f32 SquaredLength() const { return SizeSquared(); }
		f32 Length() const { return Size(); }

		b8 IsNearlyZero3(f32 tolerance = KindaSmallNumber) const { return Abs(m_x) <= tolerance && Abs(m_y) <= tolerance && Abs(m_z) <= tolerance; }
		b8 IsNearlyZero(f32 tolerance = KindaSmallNumber) const { return IsNearlyZero3(tolerance) && Abs(m_w) <= tolerance; }
		b8 IsZero3() const { return m_x == 0.f && m_y == 0.f && m_z == 0.f; }
		b8 IsZero() const { return IsZero3() && m_w == 0.f; }
		b8 IsUnit3(f32 lengthSquaredTolerance = KindaSmallNumber) const { return Abs(1.f - SizeSquared3()) < lengthSquaredTolerance; }
		b8 IsNormalized3() const { return Abs(1.f - SizeSquared3()) < ThreshVectorNormalized; }

		b8 Normalize3(f32 tolerance = SmallNumber)
		{
			const f32 squareSum = SizeSquared3();

			if (squareSum <= tolerance)
			{
				*this = Zero;
				return false;
			}

			const f32 scale = 1.f / std::sqrt(squareSum);
			m_x *= scale;
			m_y *= scale;
			m_z *= scale;
			m_w = 0.f;
			return true;
		}

		Vec4f GetSafeNormal(f32 tolerance = SmallNumber) const
		{
			const f32 squareSum = SizeSquared3();

			if (squareSum == 1.f)
			{
				if (m_w == 0.f)
					return *this;

				return Vec4f(m_x, m_y, m_z, 0.f);
			}

			if (squareSum < tolerance)
				return Zero;

			const f32 scale = 1.f / std::sqrt(squareSum);
			return Vec4f(m_x * scale, m_y * scale, m_z * scale, 0.f);
		}

		Vec4f GetUnsafeNormal3() const
		{
			const f32 scale = 1.f / Size3();
			return Vec4f(m_x * scale, m_y * scale, m_z * scale, 0.f);
		}

		f32 GetMax() const { return Max(Max(m_x, m_y), Max(m_z, m_w)); }
		f32 GetMin() const { return Min(Min(m_x, m_y), Min(m_z, m_w)); }

		f32 GetAbsMax() const { return Max(Max(Abs(m_x), Abs(m_y)), Max(Abs(m_z), Abs(m_w))); }
		f32 GetAbsMin() const { return Min(Min(Abs(m_x), Abs(m_y)), Min(Abs(m_z), Abs(m_w))); }
		Vec4f GetAbs() const { return Vec4f(Abs(m_x), Abs(m_y), Abs(m_z), Abs(m_w)); }

		Vec4f ComponentMin(const Vec4f& other) const { return Vec4f(Min(m_x, other.m_x), Min(m_y, other.m_y), Min(m_z, other.m_z), Min(m_w, other.m_w)); }
		Vec4f ComponentMax(const Vec4f& other) const { return Vec4f(Max(m_x, other.m_x), Max(m_y, other.m_y), Max(m_z, other.m_z), Max(m_w, other.m_w)); }

		Vec4f GetSignVector() const { return Vec4f(Sign(m_x), Sign(m_y), Sign(m_z), Sign(m_w)); }

		Vec4f Reciprocal() const
		{
			Vec4f result;

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

			if (m_w != 0.f)
				result.m_w = 1.f / m_w;
			else
				result.m_w = BigNumber;

			return result;
		}

		b8 ContainsNaN() const { return !std::isfinite(m_x) || !std::isfinite(m_y) || !std::isfinite(m_z) || !std::isfinite(m_w); }

		Vec4f Reflect3(const Vec4f& normal) const { return *this - normal * (2.f * Dot3(normal)); }

		void FindBestAxisVectors3(Vec4f& outAxis1, Vec4f& outAxis2) const
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

			const Vec4f temp = outAxis1 - *this * outAxis1.Dot3(*this);
			outAxis1 = temp.GetSafeNormal();
			outAxis2 = outAxis1 ^ *this;
		}

		Vec4f BoundToBox(const Vec4f& min, const Vec4f& max) const
		{
			return Vec4f(
				Clamp(m_x, min.m_x, max.m_x),
				Clamp(m_y, min.m_y, max.m_y),
				Clamp(m_z, min.m_z, max.m_z),
				Clamp(m_w, min.m_w, max.m_w));
		}

		Vec4f GetClampedToMaxSize3(f32 max) const
		{
			if (max < KindaSmallNumber)
				return Vec4f(0.f, 0.f, 0.f, m_w);

			const f32 squareSum = SizeSquared3();

			if (squareSum > max * max)
			{
				const f32 scale = max / std::sqrt(squareSum);
				return Vec4f(m_x * scale, m_y * scale, m_z * scale, m_w);
			}

			return *this;
		}

		std::string ToString() const
		{
			return "X=" + std::to_string(m_x)
				+ " Y=" + std::to_string(m_y)
				+ " Z=" + std::to_string(m_z)
				+ " W=" + std::to_string(m_w);
		}

		static f32 Dot3(const Vec4f& a, const Vec4f& b) { return a.Dot3(b); }
		static f32 Dot4(const Vec4f& a, const Vec4f& b) { return a | b; }
		static Vec4f CrossProduct3(const Vec4f& a, const Vec4f& b) { return a ^ b; }

		static f32 DistSquared3(const Vec4f& a, const Vec4f& b) { return (b - a).SizeSquared3(); }
		static f32 Dist3(const Vec4f& a, const Vec4f& b) { return std::sqrt(DistSquared3(a, b)); }
		static f32 DistSquared(const Vec4f& a, const Vec4f& b) { return (b - a).SizeSquared(); }
		static f32 Dist(const Vec4f& a, const Vec4f& b) { return std::sqrt(DistSquared(a, b)); }
		static f32 Distance(const Vec4f& a, const Vec4f& b) { return Dist(a, b); }

		static Vec4f Min(const Vec4f& a, const Vec4f& b) { return a.ComponentMin(b); }
		static Vec4f Max(const Vec4f& a, const Vec4f& b) { return a.ComponentMax(b); }
		static Vec4f Lerp(const Vec4f& a, const Vec4f& b, f32 alpha) { return a + (b - a) * alpha; }

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

	private:
		HFIELD();
		f32 m_x;

		HFIELD();
		f32 m_y;

		HFIELD();
		f32 m_z;

		HFIELD();
		f32 m_w;
	};

	inline Vec4f operator*(f32 scalar, const Vec4f& value)
	{
		return value * scalar;
	}

	inline const Vec4f Vec4f::Zero = Vec4f(0.f, 0.f, 0.f, 0.f);
	inline const Vec4f Vec4f::One = Vec4f(1.f, 1.f, 1.f, 1.f);
	inline const Vec4f Vec4f::XAxisVector = Vec4f(1.f, 0.f, 0.f, 0.f);
	inline const Vec4f Vec4f::YAxisVector = Vec4f(0.f, 1.f, 0.f, 0.f);
	inline const Vec4f Vec4f::ZAxisVector = Vec4f(0.f, 0.f, 1.f, 0.f);
	inline const Vec4f Vec4f::WAxisVector = Vec4f(0.f, 0.f, 0.f, 1.f);
}
