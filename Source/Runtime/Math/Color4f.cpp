#include "Color4f.h"

namespace Horizon::Math
{
	Color4f::Color4f() : m_r(0.f), m_g(0.f), m_b(0.f), m_a(0.f)
	{
	}

	Color4f::Color4f(f32 val) : m_r(val), m_g(val), m_b(val), m_a(val)
	{
	}

	Color4f::Color4f(f32 r, f32 g, f32 b, f32 a) : m_r(r), m_g(g), m_b(b), m_a(a)
	{
	}

	void Color4f::Set(f32 r, f32 g, f32 b, f32 a)
	{
		m_r = r;
		m_g = g;
		m_b = b;
		m_a = a;
	}

	void Color4f::Store(f32* pOut) const
	{
		pOut[0] = m_r;
		pOut[1] = m_g;
		pOut[2] = m_b;
		pOut[3] = m_a;
	}

	f32 Color4f::operator[](i32 index) const
	{
		switch (index)
		{
		case 0:  return m_r;
		case 1:  return m_g;
		case 2:  return m_b;
		default: return m_a;
		}
	}

	f32& Color4f::operator[](i32 index)
	{
		switch (index)
		{
		case 0:  return m_r;
		case 1:  return m_g;
		case 2:  return m_b;
		default: return m_a;
		}
	}

	Color4f Color4f::operator-() const
	{
		return Color4f(-m_r, -m_g, -m_b, -m_a);
	}

	Color4f Color4f::operator+(const Color4f& other) const
	{
		return Color4f(m_r + other.m_r, m_g + other.m_g, m_b + other.m_b, m_a + other.m_a);
	}

	Color4f Color4f::operator-(const Color4f& other) const
	{
		return Color4f(m_r - other.m_r, m_g - other.m_g, m_b - other.m_b, m_a - other.m_a);
	}

	Color4f Color4f::operator*(const Color4f& other) const
	{
		return Color4f(m_r * other.m_r, m_g * other.m_g, m_b * other.m_b, m_a * other.m_a);
	}

	Color4f Color4f::operator/(const Color4f& other) const
	{
		return Color4f(m_r / other.m_r, m_g / other.m_g, m_b / other.m_b, m_a / other.m_a);
	}

	Color4f Color4f::operator+(f32 scalar) const
	{
		return Color4f(m_r + scalar, m_g + scalar, m_b + scalar, m_a + scalar);
	}

	Color4f Color4f::operator-(f32 scalar) const
	{
		return Color4f(m_r - scalar, m_g - scalar, m_b - scalar, m_a - scalar);
	}

	Color4f Color4f::operator*(f32 scalar) const
	{
		return Color4f(m_r * scalar, m_g * scalar, m_b * scalar, m_a * scalar);
	}

	Color4f Color4f::operator/(f32 scalar) const
	{
		const f32 inverse = 1.f / scalar;
		return Color4f(m_r * inverse, m_g * inverse, m_b * inverse, m_a * inverse);
	}

	Color4f& Color4f::operator+=(const Color4f& other)
	{
		*this = *this + other;
		return *this;
	}

	Color4f& Color4f::operator-=(const Color4f& other)
	{
		*this = *this - other;
		return *this;
	}

	Color4f& Color4f::operator*=(const Color4f& other)
	{
		*this = *this * other;
		return *this;
	}

	Color4f& Color4f::operator/=(const Color4f& other)
	{
		*this = *this / other;
		return *this;
	}

	Color4f& Color4f::operator+=(f32 scalar)
	{
		*this = *this + scalar;
		return *this;
	}

	Color4f& Color4f::operator-=(f32 scalar)
	{
		*this = *this - scalar;
		return *this;
	}

	Color4f& Color4f::operator*=(f32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	Color4f& Color4f::operator/=(f32 scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	f32 Color4f::operator|(const Color4f& other) const
	{
		return m_r * other.m_r + m_g * other.m_g + m_b * other.m_b + m_a * other.m_a;
	}

	b8 Color4f::operator==(const Color4f& other) const
	{
		return m_r == other.m_r && m_g == other.m_g && m_b == other.m_b && m_a == other.m_a;
	}

	b8 Color4f::operator!=(const Color4f& other) const
	{
		return !(*this == other);
	}
}