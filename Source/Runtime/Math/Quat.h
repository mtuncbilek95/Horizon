#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Quat : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Quat);
	public:
		static constexpr f32 SmallNumber = 1.e-8f;
		static constexpr f32 KindaSmallNumber = 1.e-4f;
		static constexpr f32 ThreshQuatNormalized = 0.01f;

		static Quat Identity() { return Quat(0.f, 0.f, 0.f, 1.f); }
		static Quat FromAxisAngle(const Vec3f& axis, f32 angleRadians);

	public:
		Quat();
		Quat(f32 x, f32 y, f32 z, f32 w);
		~Quat() = default;

		Quat(const Quat&) = default;
		Quat& operator=(const Quat&) = default;

		Quat(Quat&& other) = default;
		Quat& operator=(Quat&&) = default;

		f32 X() const { return m_x; }
		f32 Y() const { return m_y; }
		f32 Z() const { return m_z; }
		f32 W() const { return m_w; }

		void Set(f32 x, f32 y, f32 z, f32 w);
		void Store(f32* pOut) const;
		f32 operator[](i32 index) const;

		Quat operator-() const;
		Quat operator+(const Quat& other) const;
		Quat operator-(const Quat& other) const;
		Quat operator*(const Quat& other) const;
		Quat operator*(f32 scalar) const;
		Vec3f operator*(const Vec3f& value) const;
		Quat& operator+=(const Quat& other);
		Quat& operator-=(const Quat& other);
		Quat& operator*=(const Quat& other);
		Quat& operator*=(f32 scalar);
		f32 operator|(const Quat& other) const;

		b8 operator==(const Quat& other) const;
		b8 operator!=(const Quat& other) const;
		
		f32 SizeSquared() const;
		f32 Size() const;
		b8 IsNormalized() const;

		void Normalize();
		Quat GetNormalized() const;

		Quat Conjugate() const;
		Quat Inverse() const;

		Vec3f RotateVector(const Vec3f& value) const;
		Vec3f UnrotateVector(const Vec3f& value) const;

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