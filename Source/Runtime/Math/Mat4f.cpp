#include "Mat4f.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon::Math
{
	namespace
	{
		Vec3f Normalized(const Vec3f& value)
		{
			const f32 squared = value | value;

			if (squared <= SmallNumber)
				return Vec3f::Zero();

			return value / std::sqrt(squared);
		}
	}

	Mat4f Mat4f::Translation(const Vec3f& translation)
	{
		Mat4f result = Identity();
		result.m_data[12] = translation.X();
		result.m_data[13] = translation.Y();
		result.m_data[14] = translation.Z();
		return result;
	}

	Mat4f Mat4f::Scale(const Vec3f& scale)
	{
		Mat4f result = Identity();
		result.m_data[0] = scale.X();
		result.m_data[5] = scale.Y();
		result.m_data[10] = scale.Z();
		return result;
	}

	Mat4f Mat4f::Rotation(const Quat& rotation)
	{
		const f32 x = rotation.X();
		const f32 y = rotation.Y();
		const f32 z = rotation.Z();
		const f32 w = rotation.W();

		const f32 xx = x * x * 2.f;
		const f32 yy = y * y * 2.f;
		const f32 zz = z * z * 2.f;
		const f32 xy = x * y * 2.f;
		const f32 xz = x * z * 2.f;
		const f32 yz = y * z * 2.f;
		const f32 wx = w * x * 2.f;
		const f32 wy = w * y * 2.f;
		const f32 wz = w * z * 2.f;

		Mat4f result = Identity();
		result.m_data[0] = 1.f - (yy + zz);
		result.m_data[1] = xy + wz;
		result.m_data[2] = xz - wy;
		result.m_data[4] = xy - wz;
		result.m_data[5] = 1.f - (xx + zz);
		result.m_data[6] = yz + wx;
		result.m_data[8] = xz + wy;
		result.m_data[9] = yz - wx;
		result.m_data[10] = 1.f - (xx + yy);
		return result;
	}

	Mat4f Mat4f::TRS(const Vec3f& translation, const Quat& rotation, const Vec3f& scale)
	{
		Mat4f result = Rotation(rotation);

		for (i32 col = 0; col < 3; col++)
		{
			for (i32 row = 0; row < 3; row++)
				result.m_data[col * 4 + row] *= scale[col];
		}

		result.m_data[12] = translation.X();
		result.m_data[13] = translation.Y();
		result.m_data[14] = translation.Z();
		return result;
	}

	Mat4f Mat4f::Perspective(f32 fovYRadians, f32 aspect, f32 nearZ, f32 farZ)
	{
		const f32 focal = 1.f / std::tan(fovYRadians * 0.5f);
		const f32 range = 1.f / (nearZ - farZ);

		Mat4f result = Zero();
		result.m_data[0] = focal / aspect;
		result.m_data[5] = focal;
		result.m_data[10] = farZ * range;
		result.m_data[11] = -1.f;
		result.m_data[14] = nearZ * farZ * range;
		return result;
	}

	Mat4f Mat4f::Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ)
	{
		Mat4f result = Identity();
		result.m_data[0] = 2.f / (right - left);
		result.m_data[5] = 2.f / (top - bottom);
		result.m_data[10] = -1.f / (farZ - nearZ);
		result.m_data[12] = -(right + left) / (right - left);
		result.m_data[13] = -(top + bottom) / (top - bottom);
		result.m_data[14] = -nearZ / (farZ - nearZ);
		return result;
	}

	Mat4f Mat4f::LookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up)
	{
		const Vec3f forward = Normalized(target - eye);

		if (forward == Vec3f::Zero())
		{
			Terminal::Error("Mat4f", "LookAt called with eye and target at the same position");
			return Identity();
		}

		const Vec3f side = Normalized(forward ^ up);
		const Vec3f upper = side ^ forward;

		Mat4f result = Identity();
		result.m_data[0] = side.X();
		result.m_data[4] = side.Y();
		result.m_data[8] = side.Z();
		result.m_data[1] = upper.X();
		result.m_data[5] = upper.Y();
		result.m_data[9] = upper.Z();
		result.m_data[2] = -forward.X();
		result.m_data[6] = -forward.Y();
		result.m_data[10] = -forward.Z();
		result.m_data[12] = -(side | eye);
		result.m_data[13] = -(upper | eye);
		result.m_data[14] = forward | eye;
		return result;
	}

	Mat4f::Mat4f() : Mat4f(1.f)
	{
	}

	Mat4f::Mat4f(f32 diagonal)
	{
		for (i32 i = 0; i < 16; i++)
			m_data[i] = 0.f;

		m_data[0] = diagonal;
		m_data[5] = diagonal;
		m_data[10] = diagonal;
		m_data[15] = diagonal;
	}

	Mat4f::Mat4f(const Vec4f& c0, const Vec4f& c1, const Vec4f& c2, const Vec4f& c3)
	{
		c0.Store(m_data);
		c1.Store(m_data + 4);
		c2.Store(m_data + 8);
		c3.Store(m_data + 12);
	}

	void Mat4f::Store(f32* pOut) const
	{
		for (i32 i = 0; i < 16; i++)
			pOut[i] = m_data[i];
	}

	Vec4f Mat4f::Column(i32 index) const
	{
		const f32* column = m_data + index * 4;
		return Vec4f(column[0], column[1], column[2], column[3]);
	}

	Vec4f Mat4f::Row(i32 index) const
	{
		return Vec4f(m_data[index], m_data[index + 4], m_data[index + 8], m_data[index + 12]);
	}

	void Mat4f::SetColumn(i32 index, const Vec4f& value)
	{
		value.Store(m_data + index * 4);
	}

	Mat4f Mat4f::operator*(const Mat4f& other) const
	{
		Mat4f result = Zero();

		for (i32 col = 0; col < 4; col++)
		{
			for (i32 row = 0; row < 4; row++)
			{
				f32 sum = 0.f;

				for (i32 k = 0; k < 4; k++)
					sum += m_data[k * 4 + row] * other.m_data[col * 4 + k];

				result.m_data[col * 4 + row] = sum;
			}
		}

		return result;
	}

	Vec4f Mat4f::operator*(const Vec4f& value) const
	{
		const f32 x = value.X();
		const f32 y = value.Y();
		const f32 z = value.Z();
		const f32 w = value.W();

		return Vec4f(
			m_data[0] * x + m_data[4] * y + m_data[8] * z + m_data[12] * w,
			m_data[1] * x + m_data[5] * y + m_data[9] * z + m_data[13] * w,
			m_data[2] * x + m_data[6] * y + m_data[10] * z + m_data[14] * w,
			m_data[3] * x + m_data[7] * y + m_data[11] * z + m_data[15] * w);
	}

	Mat4f Mat4f::operator*(f32 scalar) const
	{
		Mat4f result = *this;

		for (i32 i = 0; i < 16; i++)
			result.m_data[i] *= scalar;

		return result;
	}

	Mat4f& Mat4f::operator*=(const Mat4f& other)
	{
		*this = *this * other;
		return *this;
	}

	Mat4f& Mat4f::operator*=(f32 scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	b8 Mat4f::operator==(const Mat4f& other) const
	{
		for (i32 i = 0; i < 16; i++)
		{
			if (m_data[i] != other.m_data[i])
				return false;
		}

		return true;
	}

	b8 Mat4f::operator!=(const Mat4f& other) const
	{
		return !(*this == other);
	}

	Mat4f Mat4f::Transposed() const
	{
		Mat4f result = Zero();

		for (i32 col = 0; col < 4; col++)
		{
			for (i32 row = 0; row < 4; row++)
				result.m_data[row * 4 + col] = m_data[col * 4 + row];
		}

		return result;
	}

	f32 Mat4f::Determinant() const
	{
		const f32* m = m_data;

		const f32 s0 = m[0] * m[5] - m[1] * m[4];
		const f32 s1 = m[0] * m[6] - m[2] * m[4];
		const f32 s2 = m[0] * m[7] - m[3] * m[4];
		const f32 s3 = m[1] * m[6] - m[2] * m[5];
		const f32 s4 = m[1] * m[7] - m[3] * m[5];
		const f32 s5 = m[2] * m[7] - m[3] * m[6];
		const f32 c0 = m[8] * m[13] - m[9] * m[12];
		const f32 c1 = m[8] * m[14] - m[10] * m[12];
		const f32 c2 = m[8] * m[15] - m[11] * m[12];
		const f32 c3 = m[9] * m[14] - m[10] * m[13];
		const f32 c4 = m[9] * m[15] - m[11] * m[13];
		const f32 c5 = m[10] * m[15] - m[11] * m[14];

		return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
	}

	Mat4f Mat4f::Inverse() const
	{
		const f32* m = m_data;

		const f32 s0 = m[0] * m[5] - m[1] * m[4];
		const f32 s1 = m[0] * m[6] - m[2] * m[4];
		const f32 s2 = m[0] * m[7] - m[3] * m[4];
		const f32 s3 = m[1] * m[6] - m[2] * m[5];
		const f32 s4 = m[1] * m[7] - m[3] * m[5];
		const f32 s5 = m[2] * m[7] - m[3] * m[6];
		const f32 c0 = m[8] * m[13] - m[9] * m[12];
		const f32 c1 = m[8] * m[14] - m[10] * m[12];
		const f32 c2 = m[8] * m[15] - m[11] * m[12];
		const f32 c3 = m[9] * m[14] - m[10] * m[13];
		const f32 c4 = m[9] * m[15] - m[11] * m[13];
		const f32 c5 = m[10] * m[15] - m[11] * m[14];

		const f32 determinant = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

		if (IsNearlyZero(determinant, SmallNumber))
		{
			Terminal::Error("Mat4f", "Inverse called on a singular matrix");
			return Identity();
		}

		const f32 inverse = 1.f / determinant;

		Mat4f result = Zero();
		result.m_data[0] = (m[5] * c5 - m[6] * c4 + m[7] * c3) * inverse;
		result.m_data[1] = (m[2] * c4 - m[1] * c5 - m[3] * c3) * inverse;
		result.m_data[2] = (m[13] * s5 - m[14] * s4 + m[15] * s3) * inverse;
		result.m_data[3] = (m[10] * s4 - m[9] * s5 - m[11] * s3) * inverse;
		result.m_data[4] = (m[6] * c2 - m[4] * c5 - m[7] * c1) * inverse;
		result.m_data[5] = (m[0] * c5 - m[2] * c2 + m[3] * c1) * inverse;
		result.m_data[6] = (m[14] * s2 - m[12] * s5 - m[15] * s1) * inverse;
		result.m_data[7] = (m[8] * s5 - m[10] * s2 + m[11] * s1) * inverse;
		result.m_data[8] = (m[4] * c4 - m[5] * c2 + m[7] * c0) * inverse;
		result.m_data[9] = (m[1] * c2 - m[0] * c4 - m[3] * c0) * inverse;
		result.m_data[10] = (m[12] * s4 - m[13] * s2 + m[15] * s0) * inverse;
		result.m_data[11] = (m[9] * s2 - m[8] * s4 - m[11] * s0) * inverse;
		result.m_data[12] = (m[5] * c1 - m[4] * c3 - m[6] * c0) * inverse;
		result.m_data[13] = (m[0] * c3 - m[1] * c1 + m[2] * c0) * inverse;
		result.m_data[14] = (m[13] * s1 - m[12] * s3 - m[14] * s0) * inverse;
		result.m_data[15] = (m[8] * s3 - m[9] * s1 + m[10] * s0) * inverse;
		return result;
	}

	Vec3f Mat4f::TransformPoint(const Vec3f& point) const
	{
		const Vec4f result = *this * Vec4f(point.X(), point.Y(), point.Z(), 1.f);
		return Vec3f(result.X(), result.Y(), result.Z());
	}

	Vec3f Mat4f::TransformVector(const Vec3f& vector) const
	{
		const Vec4f result = *this * Vec4f(vector.X(), vector.Y(), vector.Z(), 0.f);
		return Vec3f(result.X(), result.Y(), result.Z());
	}

	Vec3f Mat4f::GetTranslation() const
	{
		return Vec3f(m_data[12], m_data[13], m_data[14]);
	}
}