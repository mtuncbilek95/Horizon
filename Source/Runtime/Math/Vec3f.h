#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec3f final
	{
		HORIZON_PRIMITIVE_REFLECT(Vec3f);
	public:
		static Vec3f Zero() { return Vec3f(0.f, 0.f, 0.f); }
		static Vec3f One() { return Vec3f(1.f, 1.f, 1.f); }

	public:
		Vec3f();
		Vec3f(f32 val);
		Vec3f(f32 x, f32 y, f32 z);
		~Vec3f() = default;

		Vec3f(const Vec3f&) = default;
		Vec3f& operator=(const Vec3f&) = default;

		Vec3f(Vec3f&& other) = default;
		Vec3f& operator=(Vec3f&&) = default;

		f32 X() const { return m_x; }
		f32& X() { return m_x; }

		f32 Y() const { return m_y; }
		f32& Y() { return m_y; }

		f32 Z() const { return m_z; }
		f32& Z() { return m_z; }

		void Set(f32 x, f32 y, f32 z);
		void Store(f32* pOut) const;
		f32 operator[](i32 index) const;
		f32& operator[](i32 index);

		Vec3f operator-() const;
		Vec3f operator+(const Vec3f& other) const;
		Vec3f operator-(const Vec3f& other) const;
		Vec3f operator*(const Vec3f& other) const;
		Vec3f operator/(const Vec3f& other) const;
		Vec3f operator+(f32 scalar) const;
		Vec3f operator-(f32 scalar) const;
		Vec3f operator*(f32 scalar) const;
		Vec3f operator/(f32 scalar) const;
		Vec3f& operator+=(const Vec3f& other);
		Vec3f& operator-=(const Vec3f& other);
		Vec3f& operator*=(const Vec3f& other);
		Vec3f& operator/=(const Vec3f& other);
		Vec3f& operator+=(f32 scalar);
		Vec3f& operator-=(f32 scalar);
		Vec3f& operator*=(f32 scalar);
		Vec3f& operator/=(f32 scalar);
		f32 operator|(const Vec3f& other) const;
		Vec3f operator^(const Vec3f& other) const;

		b8 operator==(const Vec3f& other) const;
		b8 operator!=(const Vec3f& other) const;

	private:
		HFIELD();
		f32 m_x;

		HFIELD();
		f32 m_y;

		HFIELD();
		f32 m_z;
	};
}
