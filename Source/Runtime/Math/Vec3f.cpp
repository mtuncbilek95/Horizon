#include "Vec3f.h"

namespace Horizon::Math
{
	Vec3f::Vec3f() : m_x(0.f), m_y(0.f), m_z(0.f)
	{
	}

	Vec3f::Vec3f(f32 val) : m_x(val), m_y(val), m_z(val)
	{
	}

	Vec3f::Vec3f(f32 x, f32 y, f32 z) : m_x(x), m_y(y), m_z(z)
	{
	}

	void Vec3f::Set(f32 x, f32 y, f32 z)
	{
		m_x = x;
		m_y = y;
		m_z = z;
	}

	void Vec3f::Store(f32* pOut) const
	{
		pOut[0] = m_x;
		pOut[1] = m_y;
		pOut[2] = m_z;
	}

	f32 Vec3f::operator[](i32 index) const
	{
		switch (index)
		{
		case 0:  return m_x;
		case 1:  return m_y;
		default: return m_z;
		}
	}

	f32& Vec3f::operator[](i32 index)
	{
		switch (index)
		{
		case 0:  return m_x;
		case 1:  return m_y;
		default: return m_z;
		}
	}

	Vec3f Vec3f::operator-() const
	{
		return Vec3f(-m_x, -m_y, -m_z);
	}

	Vec3f Vec3f::operator+(const Vec3f& other) const
	{
		return Vec3f(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
	}

	Vec3f Vec3f::operator-(const Vec3f& other) const
	{
		return Vec3f(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
	}

	Vec3f Vec3f::operator*(const Vec3f& other) const
	{
		return Vec3f(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z);
	}

	Vec3f Vec3f::operator/(const Vec3f& other) const
	{
		return Vec3f(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z);
	}

	Vec3f Vec3f::operator+(f32 scalar) const
	{
		return Vec3f(m_x + scalar, m_y + scalar, m_z + scalar);
	}

	Vec3f Vec3f::operator-(f32 scalar) const
	{
		return Vec3f(m_x - scalar, m_y - scalar, m_z - scalar);
	}

	Vec3f Vec3f::operator*(f32 scalar) const
	{
		return Vec3f(m_x * scalar, m_y * scalar, m_z * scalar);
	}

	Vec3f Vec3f::operator/(f32 scalar) const
	{
		const f32 inverse = 1.f / scalar;
		return Vec3f(m_x * inverse, m_y * inverse, m_z * inverse);
	}

	Vec3f& Vec3f::operator+=(const Vec3f& other)
	{
		*this = *this + other;
		return *this;
	}

	Vec3f& Vec3f::operator-=(const Vec3f& other)
	{
		*this = *this - other;
		return *this;
	}

	Vec3f& Vec3f::operator*=(const Vec3f& other)
	{
		*this = *this * other;
		return *this;
	}

	Vec3f& Vec3f::operator/=(const Vec3f& other)
	{
		*this = *this / other;
		return *this;
	}

	Vec3f& Vec3f::operator+=(f32 scalar)
	{
		*this = *this + scalar;
		return *this;
	}

	Vec3f& Vec3f::operator-=(f32 scalar)
	{
		*this = *this - scalar;
		return *this;
	}

	Vec3f& Vec3f::operator*=(f32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Vec3f& Vec3f::operator/=(f32 scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	f32 Vec3f::operator|(const Vec3f& other) const
	{
		return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
	}

	Vec3f Vec3f::operator^(const Vec3f& other) const
	{
		return Vec3f(
			m_y * other.m_z - m_z * other.m_y,
			m_z * other.m_x - m_x * other.m_z,
			m_x * other.m_y - m_y * other.m_x);
	}

	b8 Vec3f::operator==(const Vec3f& other) const
	{
		return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
	}

	b8 Vec3f::operator!=(const Vec3f& other) const
	{
		return !(*this == other);
	}
}
