#include "Vec2u.h"

namespace Horizon::Math
{
	Vec2u::Vec2u() : m_x(0.f), m_y(0.f)
	{
	}

	Vec2u::Vec2u(u32 val) : m_x(val), m_y(val)
	{
	}

	Vec2u::Vec2u(u32 x, u32 y) : m_x(x), m_y(y)
	{
	}

	void Vec2u::Set(u32 x, u32 y)
	{
		m_x = x;
		m_y = y;
	}

	void Vec2u::Store(u32* pOut) const
	{
		pOut[0] = m_x;
		pOut[1] = m_y;
	}

	u32 Vec2u::operator[](i32 index) const
	{
		switch (index)
		{
		case 0:  return m_x;
		default: return m_y;
		}
	}

	u32& Vec2u::operator[](i32 index)
	{
		switch (index)
		{
		case 0:  return m_x;
		default: return m_y;
		}
	}

	Vec2u Vec2u::operator-() const
	{
		return Vec2u(-m_x, -m_y);
	}

	Vec2u Vec2u::operator+(const Vec2u& other) const
	{
		return Vec2u(m_x + other.m_x, m_y + other.m_y);
	}

	Vec2u Vec2u::operator-(const Vec2u& other) const
	{
		return Vec2u(m_x - other.m_x, m_y - other.m_y);
	}

	Vec2u Vec2u::operator*(const Vec2u& other) const
	{
		return Vec2u(m_x * other.m_x, m_y * other.m_y);
	}

	Vec2u Vec2u::operator/(const Vec2u& other) const
	{
		return Vec2u(m_x / other.m_x, m_y / other.m_y);
	}

	Vec2u Vec2u::operator+(u32 scalar) const
	{
		return Vec2u(m_x + scalar, m_y + scalar);
	}

	Vec2u Vec2u::operator-(u32 scalar) const
	{
		return Vec2u(m_x - scalar, m_y - scalar);
	}

	Vec2u Vec2u::operator*(u32 scalar) const
	{
		return Vec2u(m_x * scalar, m_y * scalar);
	}

	Vec2u Vec2u::operator/(u32 scalar) const
	{
		const u32 inverse = 1.f / scalar;
		return Vec2u(m_x * inverse, m_y * inverse);
	}

	Vec2u& Vec2u::operator+=(const Vec2u& other)
	{
		*this = *this + other;
		return *this;
	}

	Vec2u& Vec2u::operator-=(const Vec2u& other)
	{
		*this = *this - other;
		return *this;
	}

	Vec2u& Vec2u::operator*=(const Vec2u& other)
	{
		*this = *this * other;
		return *this;
	}

	Vec2u& Vec2u::operator/=(const Vec2u& other)
	{
		*this = *this / other;
		return *this;
	}

	Vec2u& Vec2u::operator+=(u32 scalar)
	{
		*this = *this + scalar;
		return *this;
	}

	Vec2u& Vec2u::operator-=(u32 scalar)
	{
		*this = *this - scalar;
		return *this;
	}

	Vec2u& Vec2u::operator*=(u32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Vec2u& Vec2u::operator/=(u32 scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	u32 Vec2u::operator|(const Vec2u& other) const
	{
		return m_x * other.m_x + m_y * other.m_y;
	}

	u32 Vec2u::operator^(const Vec2u& other) const
	{
		return m_x * other.m_y - m_y * other.m_x;
	}

	b8 Vec2u::operator==(const Vec2u& other) const
	{
		return m_x == other.m_x && m_y == other.m_y;
	}

	b8 Vec2u::operator!=(const Vec2u& other) const
	{
		return !(*this == other);
	}
}