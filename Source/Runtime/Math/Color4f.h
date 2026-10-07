#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class RUNTIME_API Color4f final
	{
		HORIZON_PRIMITIVE_REFLECT(Color4f);
	public:
		static Color4f Black() { return Color4f(0.f, 0.f, 0.f, 1.f); }
		static Color4f White() { return Color4f(1.f, 1.f, 1.f, 1.f); }
		static Color4f Gray() { return Color4f(0.5f, 0.5f, 0.5f, 1.f); }

	public:
		Color4f();
		Color4f(f32 val);
		Color4f(f32 r, f32 g, f32 b, f32 a);
		~Color4f() = default;

		Color4f(const Color4f&) = default;
		Color4f& operator=(const Color4f&) = default;

		Color4f(Color4f&& other) = default;
		Color4f& operator=(Color4f&&) = default;

		f32 R() const { return m_r; }
		f32& R() { return m_r; }

		f32 G() const { return m_g; }
		f32& G() { return m_g; }

		f32 B() const { return m_b; }
		f32& B() { return m_b; }

		f32 A() const { return m_a; }
		f32& A() { return m_a; }

		void Set(f32 x, f32 y, f32 z, f32 w);
		void Store(f32* pOut) const;
		f32 operator[](i32 index) const;
		f32& operator[](i32 index);

		Color4f operator-() const;
		Color4f operator+(const Color4f& other) const;
		Color4f operator-(const Color4f& other) const;
		Color4f operator*(const Color4f& other) const;
		Color4f operator/(const Color4f& other) const;
		Color4f operator+(f32 scalar) const;
		Color4f operator-(f32 scalar) const;
		Color4f operator*(f32 scalar) const;
		Color4f operator/(f32 scalar) const;
		Color4f& operator+=(const Color4f& other);
		Color4f& operator-=(const Color4f& other);
		Color4f& operator*=(const Color4f& other);
		Color4f& operator/=(const Color4f& other);
		Color4f& operator+=(f32 scalar);
		Color4f& operator-=(f32 scalar);
		Color4f& operator*=(f32 scalar);
		Color4f& operator/=(f32 scalar);
		f32 operator|(const Color4f& other) const;

		b8 operator==(const Color4f& other) const;
		b8 operator!=(const Color4f& other) const;

	private:
		HFIELD();
		f32 m_r;

		HFIELD();
		f32 m_g;

		HFIELD();
		f32 m_b;

		HFIELD();
		f32 m_a;
	};
}
