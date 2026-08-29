#include "Vec4f.h"

namespace Horizon::Math
{
	Vec4f::Vec4f() : m_x(0.f), m_y(0.f), m_z(0.f), m_w(0.f)
	{
	}

	Vec4f::Vec4f(f32 val) : m_x(val), m_y(val), m_z(val), m_w(val)
	{
	}

	Vec4f::Vec4f(f32 x, f32 y, f32 z, f32 w) : m_x(x), m_y(y), m_z(z), m_w(w)
	{
	}

	void Vec4f::Set(f32 x, f32 y, f32 z, f32 w)
	{
		m_x = x;
		m_y = y;
		m_z = z;
		m_w = w;
	}

	void Vec4f::Store(f32* pOut) const
	{
		pOut[0] = m_x;
		pOut[1] = m_y;
		pOut[2] = m_z;
		pOut[3] = m_w;
	}

	f32 Vec4f::operator[](i32 index) const
	{
		switch (index)
		{
		case 0:  return m_x;
		case 1:  return m_y;
		case 2:  return m_z;
		default: return m_w;
		}
	}

	f32& Vec4f::operator[](i32 index)
	{
		switch (index)
		{
		case 0:  return m_x;
		case 1:  return m_y;
		case 2:  return m_z;
		default: return m_w;
		}
	}

	Vec4f Vec4f::operator-() const
	{
		return Vec4f(-m_x, -m_y, -m_z, -m_w);
	}

	Vec4f Vec4f::operator+(const Vec4f& other) const
	{
		return Vec4f(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
	}

	Vec4f Vec4f::operator-(const Vec4f& other) const
	{
		return Vec4f(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w);
	}

	Vec4f Vec4f::operator*(const Vec4f& other) const
	{
		return Vec4f(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z, m_w * other.m_w);
	}

	Vec4f Vec4f::operator/(const Vec4f& other) const
	{
		return Vec4f(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z, m_w / other.m_w);
	}

	Vec4f Vec4f::operator+(f32 scalar) const
	{
		return Vec4f(m_x + scalar, m_y + scalar, m_z + scalar, m_w + scalar);
	}

	Vec4f Vec4f::operator-(f32 scalar) const
	{
		return Vec4f(m_x - scalar, m_y - scalar, m_z - scalar, m_w - scalar);
	}

	Vec4f Vec4f::operator*(f32 scalar) const
	{
		return Vec4f(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar);
	}

	Vec4f Vec4f::operator/(f32 scalar) const
	{
		const f32 inverse = 1.f / scalar;
		return Vec4f(m_x * inverse, m_y * inverse, m_z * inverse, m_w * inverse);
	}

	Vec4f& Vec4f::operator+=(const Vec4f& other)
	{
		*this = *this + other;
		return *this;
	}

	Vec4f& Vec4f::operator-=(const Vec4f& other)
	{
		*this = *this - other;
		return *this;
	}

	Vec4f& Vec4f::operator*=(const Vec4f& other)
	{
		*this = *this * other;
		return *this;
	}

	Vec4f& Vec4f::operator/=(const Vec4f& other)
	{
		*this = *this / other;
		return *this;
	}

	Vec4f& Vec4f::operator+=(f32 scalar)
	{
		*this = *this + scalar;
		return *this;
	}

	Vec4f& Vec4f::operator-=(f32 scalar)
	{
		*this = *this - scalar;
		return *this;
	}

	Vec4f& Vec4f::operator*=(f32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Vec4f& Vec4f::operator/=(f32 scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	f32 Vec4f::operator|(const Vec4f& other) const
	{
		return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z + m_w * other.m_w;
	}

	b8 Vec4f::operator==(const Vec4f& other) const
	{
		return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z && m_w == other.m_w;
	}

	b8 Vec4f::operator!=(const Vec4f& other) const
	{
		return !(*this == other);
	}
}