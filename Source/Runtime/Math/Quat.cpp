#include "Quat.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon::Math
{
	Quat Quat::FromAxisAngle(const Vec3f& axis, f32 angleRadians)
	{
		const f32 half = angleRadians * 0.5f;
		const f32 sine = std::sin(half);

		return Quat(axis.X() * sine, axis.Y() * sine, axis.Z() * sine, std::cos(half));
	}

	Quat::Quat() : m_x(0.f), m_y(0.f), m_z(0.f), m_w(1.f)
	{
	}

	Quat::Quat(f32 x, f32 y, f32 z, f32 w) : m_x(x), m_y(y), m_z(z), m_w(w)
	{
	}

	void Quat::Set(f32 x, f32 y, f32 z, f32 w)
	{
		m_x = x;
		m_y = y;
		m_z = z;
		m_w = w;
	}

	void Quat::Store(f32* pOut) const
	{
		pOut[0] = m_x;
		pOut[1] = m_y;
		pOut[2] = m_z;
		pOut[3] = m_w;
	}

	f32 Quat::operator[](i32 index) const
	{
		switch (index)
		{
		case 0:  return m_x;
		case 1:  return m_y;
		case 2:  return m_z;
		default: return m_w;
		}
	}

	Quat Quat::operator-() const
	{
		return Quat(-m_x, -m_y, -m_z, -m_w);
	}

	Quat Quat::operator+(const Quat& other) const
	{
		return Quat(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
	}

	Quat Quat::operator-(const Quat& other) const
	{
		return Quat(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w);
	}

	Quat Quat::operator*(const Quat& other) const
	{
		return Quat(
			m_w * other.m_x + m_x * other.m_w + m_y * other.m_z - m_z * other.m_y,
			m_w * other.m_y - m_x * other.m_z + m_y * other.m_w + m_z * other.m_x,
			m_w * other.m_z + m_x * other.m_y - m_y * other.m_x + m_z * other.m_w,
			m_w * other.m_w - m_x * other.m_x - m_y * other.m_y - m_z * other.m_z);
	}

	Quat Quat::operator*(f32 scalar) const
	{
		return Quat(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar);
	}

	Vec3f Quat::operator*(const Vec3f& value) const
	{
		return RotateVector(value);
	}

	Quat& Quat::operator+=(const Quat& other)
	{
		*this = *this + other;
		return *this;
	}

	Quat& Quat::operator-=(const Quat& other)
	{
		*this = *this - other;
		return *this;
	}

	Quat& Quat::operator*=(const Quat& other)
	{
		*this = *this * other;
		return *this;
	}

	Quat& Quat::operator*=(f32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	f32 Quat::operator|(const Quat& other) const
	{
		return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z + m_w * other.m_w;
	}

	b8 Quat::operator==(const Quat& other) const
	{
		return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z && m_w == other.m_w;
	}

	b8 Quat::operator!=(const Quat& other) const
	{
		return !(*this == other);
	}

	f32 Quat::SizeSquared() const
	{
		return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
	}

	f32 Quat::Size() const
	{
		return std::sqrt(SizeSquared());
	}

	b8 Quat::IsNormalized() const
	{
		const f32 delta = SizeSquared() - 1.f;
		return (delta < 0.f ? -delta : delta) < ThreshQuatNormalized;
	}

	void Quat::Normalize()
	{
		const f32 squared = SizeSquared();

		if (squared <= SmallNumber)
		{
			Terminal::Error("Quat", "Normalize called on a zero-length quaternion");
			*this = Identity();
			return;
		}

		const f32 inverse = 1.f / std::sqrt(squared);

		m_x *= inverse;
		m_y *= inverse;
		m_z *= inverse;
		m_w *= inverse;
	}

	Quat Quat::GetNormalized() const
	{
		Quat result = *this;
		result.Normalize();
		return result;
	}

	Quat Quat::Conjugate() const
	{
		return Quat(-m_x, -m_y, -m_z, m_w);
	}

	Quat Quat::Inverse() const
	{
		const f32 squared = SizeSquared();

		if (squared <= SmallNumber)
		{
			Terminal::Error("Quat", "Inverse called on a zero-length quaternion");
			return Identity();
		}

		const f32 inverse = 1.f / squared;
		return Quat(-m_x * inverse, -m_y * inverse, -m_z * inverse, m_w * inverse);
	}

	Vec3f Quat::RotateVector(const Vec3f& value) const
	{
		const Vec3f axis(m_x, m_y, m_z);
		const Vec3f cross = axis ^ value;

		return value + ((cross * m_w) + (axis ^ cross)) * 2.f;
	}

	Vec3f Quat::UnrotateVector(const Vec3f& value) const
	{
		return Conjugate().RotateVector(value);
	}
}