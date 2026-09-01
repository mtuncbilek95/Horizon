#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Vec2u : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Vec2u);
	public:
		static Vec2u Zero() { return Vec2u(0.f, 0.f); }
		static Vec2u One() { return Vec2u(1.f, 1.f); }

	public:
		Vec2u();
		Vec2u(u32 val);
		Vec2u(u32 x, u32 y);
		~Vec2u() = default;

		template<typename T>
			requires std::is_arithmetic_v<T>
		Vec2u(T x, T y) : m_x(u32(x)), m_y(u32(y))
		{
		}

		Vec2u(const Vec2u&) = default;
		Vec2u& operator=(const Vec2u&) = default;

		Vec2u(Vec2u&& other) = default;
		Vec2u& operator=(Vec2u&&) = default;

		u32 X() const { return m_x; }
		u32& X() { return m_x; }

		u32 Y() const { return m_y; }
		u32& Y() { return m_y; }

		void Set(u32 x, u32 y);
		void Store(u32* pOut) const;
		u32 operator[](i32 index) const;
		u32& operator[](i32 index);

		Vec2u operator-() const;
		Vec2u operator+(const Vec2u& other) const;
		Vec2u operator-(const Vec2u& other) const;
		Vec2u operator*(const Vec2u& other) const;
		Vec2u operator/(const Vec2u& other) const;
		Vec2u operator+(u32 scalar) const;
		Vec2u operator-(u32 scalar) const;
		Vec2u operator*(u32 scalar) const;
		Vec2u operator/(u32 scalar) const;
		Vec2u& operator+=(const Vec2u& other);
		Vec2u& operator-=(const Vec2u& other);
		Vec2u& operator*=(const Vec2u& other);
		Vec2u& operator/=(const Vec2u& other);
		Vec2u& operator+=(u32 scalar);
		Vec2u& operator-=(u32 scalar);
		Vec2u& operator*=(u32 scalar);
		Vec2u& operator/=(u32 scalar);
		u32 operator|(const Vec2u& other) const;
		u32 operator^(const Vec2u& other) const;

		b8 operator==(const Vec2u& other) const;
		b8 operator!=(const Vec2u& other) const;

	private:
		HFIELD();
		u32 m_x;

		HFIELD();
		u32 m_y;
	};
}
