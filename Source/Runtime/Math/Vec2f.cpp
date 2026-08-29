#include "Vec2f.h"

namespace Horizon::Math
{
	Vec2f::Vec2f() : m_x(0.f), m_y(0.f)
	{
	}

	Vec2f::Vec2f(f32 val) : m_x(val), m_y(val)
	{
	}

	Vec2f::Vec2f(f32 x, f32 y) : m_x(x), m_y(y)
	{
	}

	void Vec2f::Set(f32 x, f32 y)
	{
		m_x = x;
		m_y = y;
	}

	void Vec2f::Store(f32* pOut) const
	{
		pOut[0] = m_x;
		pOut[1] = m_y;
	}

	f32 Vec2f::operator[](i32 index) const
	{
		switch (index)
		{
		case 0:  return m_x;
		default: return m_y;
		}
	}

	f32& Vec2f::operator[](i32 index)
	{
		switch (index)
		{
		case 0:  return m_x;
		default: return m_y;
		}
	}

	Vec2f Vec2f::operator-() const
	{
		return Vec2f(-m_x, -m_y);
	}

	Vec2f Vec2f::operator+(const Vec2f& other) const
	{
		return Vec2f(m_x + other.m_x, m_y + other.m_y);
	}

	Vec2f Vec2f::operator-(const Vec2f& other) const
	{
		return Vec2f(m_x - other.m_x, m_y - other.m_y);
	}

	Vec2f Vec2f::operator*(const Vec2f& other) const
	{
		return Vec2f(m_x * other.m_x, m_y * other.m_y);
	}

	Vec2f Vec2f::operator/(const Vec2f& other) const
	{
		return Vec2f(m_x / other.m_x, m_y / other.m_y);
	}

	Vec2f Vec2f::operator+(f32 scalar) const
	{
		return Vec2f(m_x + scalar, m_y + scalar);
	}

	Vec2f Vec2f::operator-(f32 scalar) const
	{
		return Vec2f(m_x - scalar, m_y - scalar);
	}

	Vec2f Vec2f::operator*(f32 scalar) const
	{
		return Vec2f(m_x * scalar, m_y * scalar);
	}

	Vec2f Vec2f::operator/(f32 scalar) const
	{
		const f32 inverse = 1.f / scalar;
		return Vec2f(m_x * inverse, m_y * inverse);
	}

	Vec2f& Vec2f::operator+=(const Vec2f& other)
	{
		*this = *this + other;
		return *this;
	}

	Vec2f& Vec2f::operator-=(const Vec2f& other)
	{
		*this = *this - other;
		return *this;
	}

	Vec2f& Vec2f::operator*=(const Vec2f& other)
	{
		*this = *this * other;
		return *this;
	}

	Vec2f& Vec2f::operator/=(const Vec2f& other)
	{
		*this = *this / other;
		return *this;
	}

	Vec2f& Vec2f::operator+=(f32 scalar)
	{
		*this = *this + scalar;
		return *this;
	}

	Vec2f& Vec2f::operator-=(f32 scalar)
	{
		*this = *this - scalar;
		return *this;
	}

	Vec2f& Vec2f::operator*=(f32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Vec2f& Vec2f::operator/=(f32 scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	f32 Vec2f::operator|(const Vec2f& other) const
	{
		return m_x * other.m_x + m_y * other.m_y;
	}

	f32 Vec2f::operator^(const Vec2f& other) const
	{
		return m_x * other.m_y - m_y * other.m_x;
	}

	b8 Vec2f::operator==(const Vec2f& other) const
	{
		return m_x == other.m_x && m_y == other.m_y;
	}

	b8 Vec2f::operator!=(const Vec2f& other) const
	{
		return !(*this == other);
	}
}