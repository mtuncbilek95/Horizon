#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Vec4f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Math
{
	class H_EXPORT Mat4f
	{
	public:
		static Mat4f Identity() { return Mat4f(1.f); }
		static Mat4f Zero() { return Mat4f(0.f); }
		static Mat4f Translation(const Vec3f& translation);
		static Mat4f Scale(const Vec3f& scale);
		static Mat4f Rotation(const Quat& rotation);
		static Mat4f TRS(const Vec3f& translation, const Quat& rotation, const Vec3f& scale);
		static Mat4f Perspective(f32 fovYRadians, f32 aspect, f32 nearZ, f32 farZ);
		static Mat4f Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ);
		static Mat4f LookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up);

	public:
		Mat4f();
		Mat4f(f32 diagonal);
		Mat4f(const Vec4f& c0, const Vec4f& c1, const Vec4f& c2, const Vec4f& c3);
		~Mat4f() = default;

		Mat4f(const Mat4f&) = default;
		Mat4f& operator=(const Mat4f&) = default;

		Mat4f(Mat4f&& other) = default;
		Mat4f& operator=(Mat4f&&) = default;

		f32 operator()(i32 row, i32 col) const { return m_data[col * 4 + row]; }
		f32& operator()(i32 row, i32 col) { return m_data[col * 4 + row]; }
		f32 operator[](i32 index) const { return m_data[index]; }
		f32& operator[](i32 index) { return m_data[index]; }

		const f32* Data() const { return m_data; }
		void Store(f32* pOut) const;

		Vec4f Column(i32 index) const;
		Vec4f Row(i32 index) const;
		void SetColumn(i32 index, const Vec4f& value);

		Mat4f operator*(const Mat4f& other) const;
		Vec4f operator*(const Vec4f& value) const;
		Mat4f operator*(f32 scalar) const;
		Mat4f& operator*=(const Mat4f& other);
		Mat4f& operator*=(f32 scalar);

		b8 operator==(const Mat4f& other) const;
		b8 operator!=(const Mat4f& other) const;

		Mat4f Transposed() const;
		f32 Determinant() const;
		Mat4f Inverse() const;

		Vec3f TransformPoint(const Vec3f& point) const;
		Vec3f TransformVector(const Vec3f& vector) const;

		Vec3f GetTranslation() const;

	private:
		f32 m_data[16];
	};
}