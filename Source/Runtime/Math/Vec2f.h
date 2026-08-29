#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec2f : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Vec2f);
	public:
		static Vec2f Zero() { return Vec2f(0.f, 0.f); }
		static Vec2f One() { return Vec2f(1.f, 1.f); }

	public:
		Vec2f();
		Vec2f(f32 val);
		Vec2f(f32 x, f32 y);
		~Vec2f() = default;

		Vec2f(const Vec2f&) = default;
		Vec2f& operator=(const Vec2f&) = default;

		Vec2f(Vec2f&& other) = default;
		Vec2f& operator=(Vec2f&&) = default;

		f32 X() const { return m_x; }
		f32& X() { return m_x; }

		f32 Y() const { return m_y; }
		f32& Y() { return m_y; }

		void Set(f32 x, f32 y);
		void Store(f32* pOut) const;
		f32 operator[](i32 index) const;
		f32& operator[](i32 index);

		Vec2f operator-() const;
		Vec2f operator+(const Vec2f& other) const;
		Vec2f operator-(const Vec2f& other) const;
		Vec2f operator*(const Vec2f& other) const;
		Vec2f operator/(const Vec2f& other) const;
		Vec2f operator+(f32 scalar) const;
		Vec2f operator-(f32 scalar) const;
		Vec2f operator*(f32 scalar) const;
		Vec2f operator/(f32 scalar) const;
		Vec2f& operator+=(const Vec2f& other);
		Vec2f& operator-=(const Vec2f& other);
		Vec2f& operator*=(const Vec2f& other);
		Vec2f& operator/=(const Vec2f& other);
		Vec2f& operator+=(f32 scalar);
		Vec2f& operator-=(f32 scalar);
		Vec2f& operator*=(f32 scalar);
		Vec2f& operator/=(f32 scalar);
		f32 operator|(const Vec2f& other) const;
		f32 operator^(const Vec2f& other) const;

		b8 operator==(const Vec2f& other) const;
		b8 operator!=(const Vec2f& other) const;

	private:
		HFIELD();
		f32 m_x;

		HFIELD();
		f32 m_y;
	};
}
