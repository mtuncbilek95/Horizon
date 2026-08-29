#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec4f : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Vec4f);
	public:
		static Vec4f Zero() { return Vec4f(0.f, 0.f, 0.f, 0.f); }
		static Vec4f One() { return Vec4f(1.f, 1.f, 1.f, 1.f); }

	public:
		Vec4f();
		Vec4f(f32 val);
		Vec4f(f32 x, f32 y, f32 z, f32 w);
		~Vec4f() = default;

		Vec4f(const Vec4f&) = default;
		Vec4f& operator=(const Vec4f&) = default;

		Vec4f(Vec4f&& other) = default;
		Vec4f& operator=(Vec4f&&) = default;

		f32 X() const { return m_x; }
		f32& X() { return m_x; }

		f32 Y() const { return m_y; }
		f32& Y() { return m_y; }

		f32 Z() const { return m_z; }
		f32& Z() { return m_z; }

		f32 W() const { return m_w; }
		f32& W() { return m_w; }

		void Set(f32 x, f32 y, f32 z, f32 w);
		void Store(f32* pOut) const;
		f32 operator[](i32 index) const;
		f32& operator[](i32 index);

		Vec4f operator-() const;
		Vec4f operator+(const Vec4f& other) const;
		Vec4f operator-(const Vec4f& other) const;
		Vec4f operator*(const Vec4f& other) const;
		Vec4f operator/(const Vec4f& other) const;
		Vec4f operator+(f32 scalar) const;
		Vec4f operator-(f32 scalar) const;
		Vec4f operator*(f32 scalar) const;
		Vec4f operator/(f32 scalar) const;
		Vec4f& operator+=(const Vec4f& other);
		Vec4f& operator-=(const Vec4f& other);
		Vec4f& operator*=(const Vec4f& other);
		Vec4f& operator/=(const Vec4f& other);
		Vec4f& operator+=(f32 scalar);
		Vec4f& operator-=(f32 scalar);
		Vec4f& operator*=(f32 scalar);
		Vec4f& operator/=(f32 scalar);
		f32 operator|(const Vec4f& other) const;

		b8 operator==(const Vec4f& other) const;
		b8 operator!=(const Vec4f& other) const;
		
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
}
