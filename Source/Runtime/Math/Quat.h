#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <Runtime/Math/Vec3f.h>

#include <cmath>
#include <string>

namespace Horizon::Math
{
	HCLASS();
	class H_EXPORT Quat : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(Quat);
	public:
		static constexpr f32 SmallNumber = 1.e-8f;
		static constexpr f32 KindaSmallNumber = 1.e-4f;
		static constexpr f32 Pi = 3.1415926535897932f;
		static constexpr f32 ThreshQuatNormalized = 0.01f;
		static constexpr f32 SingularityThreshold = 0.4999995f;
		static constexpr f32 SlerpCosineThreshold = 0.9999f;

		static const Quat Identity;

		Quat() : m_x(0.f), m_y(0.f), m_z(0.f), m_w(1.f)
		{
		}

		Quat(f32 x, f32 y, f32 z, f32 w) : m_x(x), m_y(y), m_z(z), m_w(w)
		{
		}

		Quat(const Vec3f& axis, f32 angleRadians)
		{
			const f32 half = angleRadians * 0.5f;
			const f32 s = std::sin(half);
			const f32 c = std::cos(half);

			m_x = s * axis.X();
			m_y = s * axis.Y();
			m_z = s * axis.Z();
			m_w = c;
		}

		f32& X() { return m_x; }
		f32& Y() { return m_y; }
		f32& Z() { return m_z; }
		f32& W() { return m_w; }

		f32 X() const { return m_x; }
		f32 Y() const { return m_y; }
		f32 Z() const { return m_z; }
		f32 W() const { return m_w; }

		void Set(f32 x, f32 y, f32 z, f32 w)
		{
			m_x = x;
			m_y = y;
			m_z = z;
			m_w = w;
		}

		f32& operator[](i32 index)
		{
			if (index == 0)
				return m_x;

			if (index == 1)
				return m_y;

			if (index == 2)
				return m_z;

			return m_w;
		}

		f32 operator[](i32 index) const
		{
			if (index == 0)
				return m_x;

			if (index == 1)
				return m_y;

			if (index == 2)
				return m_z;

			return m_w;
		}

		Vec3f GetVector() const { return Vec3f(m_x, m_y, m_z); }

		Quat operator-() const { return Quat(-m_x, -m_y, -m_z, -m_w); }
		Quat operator+(const Quat& other) const { return Quat(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w); }
		Quat operator-(const Quat& other) const { return Quat(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w); }
		Quat operator*(f32 scalar) const { return Quat(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar); }

		Quat operator/(f32 scalar) const
		{
			const f32 inv = 1.f / scalar;
			return Quat(m_x * inv, m_y * inv, m_z * inv, m_w * inv);
		}

		Quat operator*(const Quat& other) const
		{
			return Quat(
				m_w * other.m_x + m_x * other.m_w + m_y * other.m_z - m_z * other.m_y,
				m_w * other.m_y - m_x * other.m_z + m_y * other.m_w + m_z * other.m_x,
				m_w * other.m_z + m_x * other.m_y - m_y * other.m_x + m_z * other.m_w,
				m_w * other.m_w - m_x * other.m_x - m_y * other.m_y - m_z * other.m_z);
		}

		Vec3f operator*(const Vec3f& value) const { return RotateVector(value); }

		Quat& operator+=(const Quat& other)
		{
			m_x += other.m_x;
			m_y += other.m_y;
			m_z += other.m_z;
			m_w += other.m_w;
			return *this;
		}

		Quat& operator-=(const Quat& other)
		{
			m_x -= other.m_x;
			m_y -= other.m_y;
			m_z -= other.m_z;
			m_w -= other.m_w;
			return *this;
		}

		Quat& operator*=(const Quat& other)
		{
			*this = *this * other;
			return *this;
		}

		Quat& operator*=(f32 scalar)
		{
			m_x *= scalar;
			m_y *= scalar;
			m_z *= scalar;
			m_w *= scalar;
			return *this;
		}

		Quat& operator/=(f32 scalar)
		{
			const f32 inv = 1.f / scalar;
			m_x *= inv;
			m_y *= inv;
			m_z *= inv;
			m_w *= inv;
			return *this;
		}

		f32 operator|(const Quat& other) const { return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z + m_w * other.m_w; }
		f32 Dot(const Quat& other) const { return *this | other; }

		b8 operator==(const Quat& other) const { return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z && m_w == other.m_w; }
		b8 operator!=(const Quat& other) const { return m_x != other.m_x || m_y != other.m_y || m_z != other.m_z || m_w != other.m_w; }

		b8 Equals(const Quat& other, f32 tolerance = KindaSmallNumber) const
		{
			const b8 same = Abs(m_x - other.m_x) <= tolerance
				&& Abs(m_y - other.m_y) <= tolerance
				&& Abs(m_z - other.m_z) <= tolerance
				&& Abs(m_w - other.m_w) <= tolerance;

			if (same)
				return true;

			return Abs(m_x + other.m_x) <= tolerance
				&& Abs(m_y + other.m_y) <= tolerance
				&& Abs(m_z + other.m_z) <= tolerance
				&& Abs(m_w + other.m_w) <= tolerance;
		}

		b8 IsIdentity(f32 tolerance = SmallNumber) const { return Equals(Identity, tolerance); }
		b8 ContainsNaN() const { return !std::isfinite(m_x) || !std::isfinite(m_y) || !std::isfinite(m_z) || !std::isfinite(m_w); }

		f32 SizeSquared() const { return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w; }
		f32 Size() const { return std::sqrt(SizeSquared()); }

		b8 IsNormalized() const { return Abs(1.f - SizeSquared()) < ThreshQuatNormalized; }

		void Normalize(f32 tolerance = SmallNumber)
		{
			const f32 squareSum = SizeSquared();

			if (squareSum < tolerance)
			{
				*this = Identity;
				return;
			}

			const f32 scale = 1.f / std::sqrt(squareSum);
			m_x *= scale;
			m_y *= scale;
			m_z *= scale;
			m_w *= scale;
		}

		Quat GetNormalized(f32 tolerance = SmallNumber) const
		{
			Quat result = *this;
			result.Normalize(tolerance);
			return result;
		}

		Quat Inverse() const { return Quat(-m_x, -m_y, -m_z, m_w); }

		void ToggleSign()
		{
			m_x = -m_x;
			m_y = -m_y;
			m_z = -m_z;
			m_w = -m_w;
		}

		void EnforceShortestArcWith(const Quat& other)
		{
			const f32 dotResult = other | *this;

			if (dotResult >= 0.f)
				return;

			ToggleSign();
		}

		Vec3f RotateVector(const Vec3f& value) const
		{
			const Vec3f q(m_x, m_y, m_z);
			const Vec3f t = Vec3f::CrossProduct(q, value) * 2.f;
			return value + t * m_w + Vec3f::CrossProduct(q, t);
		}

		Vec3f UnrotateVector(const Vec3f& value) const
		{
			const Vec3f q(-m_x, -m_y, -m_z);
			const Vec3f t = Vec3f::CrossProduct(q, value) * 2.f;
			return value + t * m_w + Vec3f::CrossProduct(q, t);
		}

		Vec3f GetAxisX() const { return RotateVector(Vec3f::XAxisVector); }
		Vec3f GetAxisY() const { return RotateVector(Vec3f::YAxisVector); }
		Vec3f GetAxisZ() const { return RotateVector(Vec3f::ZAxisVector); }

		Vec3f GetForwardVector() const { return GetAxisX(); }
		Vec3f GetRightVector() const { return GetAxisY(); }
		Vec3f GetUpVector() const { return GetAxisZ(); }
		Vec3f Vector() const { return GetAxisX(); }

		f32 GetAngle() const { return 2.f * std::acos(Clamp(m_w, -1.f, 1.f)); }

		Vec3f GetRotationAxis() const
		{
			const f32 squareSum = m_x * m_x + m_y * m_y + m_z * m_z;

			if (squareSum < SmallNumber)
				return Vec3f::XAxisVector;

			const f32 scale = 1.f / std::sqrt(squareSum);
			return Vec3f(m_x * scale, m_y * scale, m_z * scale);
		}

		void ToAxisAndAngle(Vec3f& outAxis, f32& outAngleRadians) const
		{
			outAngleRadians = GetAngle();
			outAxis = GetRotationAxis();
		}

		void ToSwingTwist(const Vec3f& twistAxis, Quat& outSwing, Quat& outTwist) const
		{
			const Vec3f projection = twistAxis * Vec3f::DotProduct(twistAxis, Vec3f(m_x, m_y, m_z));
			outTwist = Quat(projection.X(), projection.Y(), projection.Z(), m_w);

			if (outTwist.SizeSquared() == 0.f)
				outTwist = Identity;
			else
				outTwist.Normalize();

			outSwing = *this * outTwist.Inverse();
		}

		f32 GetTwistAngle(const Vec3f& twistAxis) const
		{
			const f32 xyz = Vec3f::DotProduct(twistAxis, Vec3f(m_x, m_y, m_z));
			return UnwindRadians(2.f * std::atan2(xyz, m_w));
		}

		f32 AngularDistance(const Quat& other) const
		{
			const f32 innerProduct = *this | other;
			return std::acos(Clamp(2.f * innerProduct * innerProduct - 1.f, -1.f, 1.f));
		}

		Quat Log() const
		{
			Quat result;
			result.m_w = 0.f;

			if (Abs(m_w) < 1.f)
			{
				const f32 angle = std::acos(m_w);
				const f32 sinAngle = std::sin(angle);

				if (Abs(sinAngle) >= SmallNumber)
				{
					const f32 scale = angle / sinAngle;
					result.m_x = scale * m_x;
					result.m_y = scale * m_y;
					result.m_z = scale * m_z;
					return result;
				}
			}

			result.m_x = m_x;
			result.m_y = m_y;
			result.m_z = m_z;
			return result;
		}

		Quat Exp() const
		{
			const f32 angle = std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
			const f32 sinAngle = std::sin(angle);

			Quat result;
			result.m_w = std::cos(angle);

			if (Abs(sinAngle) >= SmallNumber)
			{
				const f32 scale = sinAngle / angle;
				result.m_x = scale * m_x;
				result.m_y = scale * m_y;
				result.m_z = scale * m_z;
				return result;
			}

			result.m_x = m_x;
			result.m_y = m_y;
			result.m_z = m_z;
			return result;
		}

		Vec3f Euler() const
		{
			const f32 singularityTest = m_z * m_x - m_w * m_y;
			const f32 yawY = 2.f * (m_w * m_z + m_x * m_y);
			const f32 yawX = 1.f - 2.f * (m_y * m_y + m_z * m_z);

			f32 pitch = 0.f;
			f32 yaw = 0.f;
			f32 roll = 0.f;

			if (singularityTest < -SingularityThreshold)
			{
				pitch = -90.f;
				yaw = std::atan2(yawY, yawX) * (180.f / Pi);
				roll = NormalizeAxis(-yaw - 2.f * std::atan2(m_x, m_w) * (180.f / Pi));
			}
			else if (singularityTest > SingularityThreshold)
			{
				pitch = 90.f;
				yaw = std::atan2(yawY, yawX) * (180.f / Pi);
				roll = NormalizeAxis(yaw - 2.f * std::atan2(m_x, m_w) * (180.f / Pi));
			}
			else
			{
				pitch = std::asin(Clamp(2.f * singularityTest, -1.f, 1.f)) * (180.f / Pi);
				yaw = std::atan2(yawY, yawX) * (180.f / Pi);
				roll = std::atan2(-2.f * (m_w * m_x + m_y * m_z), 1.f - 2.f * (m_x * m_x + m_y * m_y)) * (180.f / Pi);
			}

			return Vec3f(roll, pitch, yaw);
		}

		Vec3f ToRotationVector() const
		{
			Quat value = *this;
			value.EnforceShortestArcWith(Identity);

			const Quat logValue = value.Log();
			return Vec3f(logValue.m_x, logValue.m_y, logValue.m_z) * 2.f;
		}

		std::string ToString() const
		{
			return "X=" + std::to_string(m_x)
				+ " Y=" + std::to_string(m_y)
				+ " Z=" + std::to_string(m_z)
				+ " W=" + std::to_string(m_w);
		}

		static Quat MakeFromEuler(const Vec3f& euler)
		{
			const f32 radsDividedBy2 = (Pi / 180.f) * 0.5f;

			const f32 roll = std::fmod(euler.X(), 360.f) * radsDividedBy2;
			const f32 pitch = std::fmod(euler.Y(), 360.f) * radsDividedBy2;
			const f32 yaw = std::fmod(euler.Z(), 360.f) * radsDividedBy2;

			const f32 sr = std::sin(roll);
			const f32 cr = std::cos(roll);
			const f32 sp = std::sin(pitch);
			const f32 cp = std::cos(pitch);
			const f32 sy = std::sin(yaw);
			const f32 cy = std::cos(yaw);

			return Quat(
				cr * sp * sy - sr * cp * cy,
				-cr * sp * cy - sr * cp * sy,
				cr * cp * sy - sr * sp * cy,
				cr * cp * cy + sr * sp * sy);
		}

		static Quat MakeFromRotationVector(const Vec3f& rotationVector)
		{
			const Quat value(rotationVector.X() * 0.5f, rotationVector.Y() * 0.5f, rotationVector.Z() * 0.5f, 0.f);
			return value.Exp();
		}

		static Quat MakeFromAxisAngle(const Vec3f& axis, f32 angleRadians) { return Quat(axis, angleRadians); }

		static f32 DotProduct(const Quat& a, const Quat& b) { return a | b; }
		static f32 Error(const Quat& a, const Quat& b)
		{
			const f32 cosom = Abs(a.m_x * b.m_x + a.m_y * b.m_y + a.m_z * b.m_z + a.m_w * b.m_w);

			if (Abs(cosom - 1.f) < 0.01f)
				return (1.f - cosom) * 2.f;

			return std::acos(Clamp(cosom, -1.f, 1.f)) * (2.f / Pi);
		}

		static f32 ErrorAutoNormalize(const Quat& a, const Quat& b) { return Error(a.GetNormalized(), b.GetNormalized()); }

		static Quat FindBetweenNormals(const Vec3f& a, const Vec3f& b) { return FindBetweenHelper(a, b, 1.f); }

		static Quat FindBetweenVectors(const Vec3f& a, const Vec3f& b)
		{
			const f32 normAB = std::sqrt(a.SizeSquared() * b.SizeSquared());
			return FindBetweenHelper(a, b, normAB);
		}

		static Quat FindBetween(const Vec3f& a, const Vec3f& b) { return FindBetweenVectors(a, b); }

		static Quat SlerpNotNormalized(const Quat& a, const Quat& b, f32 alpha)
		{
			const f32 rawCosom = a | b;
			f32 cosom = rawCosom;

			if (cosom < 0.f)
				cosom = -cosom;

			f32 scale0 = 0.f;
			f32 scale1 = 0.f;

			if (cosom < SlerpCosineThreshold)
			{
				const f32 omega = std::acos(cosom);
				const f32 invSin = 1.f / std::sin(omega);
				scale0 = std::sin((1.f - alpha) * omega) * invSin;
				scale1 = std::sin(alpha * omega) * invSin;
			}
			else
			{
				scale0 = 1.f - alpha;
				scale1 = alpha;
			}

			if (rawCosom < 0.f)
				scale1 = -scale1;

			return Quat(
				scale0 * a.m_x + scale1 * b.m_x,
				scale0 * a.m_y + scale1 * b.m_y,
				scale0 * a.m_z + scale1 * b.m_z,
				scale0 * a.m_w + scale1 * b.m_w);
		}

		static Quat Slerp(const Quat& a, const Quat& b, f32 alpha) { return SlerpNotNormalized(a, b, alpha).GetNormalized(); }

		static Quat SlerpFullPathNotNormalized(const Quat& a, const Quat& b, f32 alpha)
		{
			const f32 cosAngle = Clamp(a | b, -1.f, 1.f);
			const f32 angle = std::acos(cosAngle);

			if (Abs(angle) < KindaSmallNumber)
				return a;

			const f32 invSinAngle = 1.f / std::sin(angle);
			return a * (std::sin((1.f - alpha) * angle) * invSinAngle) + b * (std::sin(alpha * angle) * invSinAngle);
		}

		static Quat SlerpFullPath(const Quat& a, const Quat& b, f32 alpha) { return SlerpFullPathNotNormalized(a, b, alpha).GetNormalized(); }
		static Quat FastLerp(const Quat& a, const Quat& b, f32 alpha) { return (b * alpha + a * (1.f - alpha)).GetNormalized(); }
		static Quat FastBilerp(const Quat& p00, const Quat& p10, const Quat& p01, const Quat& p11, f32 fracX, f32 fracY) 
		{
			return FastLerp(FastLerp(p00, p10, fracX), FastLerp(p01, p11, fracX), fracY);
		}

		static Quat Squad(const Quat& quat1, const Quat& tangent1, const Quat& quat2, const Quat& tangent2, f32 alpha)
		{
			const Quat q1 = SlerpNotNormalized(quat1, quat2, alpha);
			const Quat q2 = SlerpFullPathNotNormalized(tangent1, tangent2, alpha);
			return SlerpFullPath(q1, q2, 2.f * alpha * (1.f - alpha));
		}

		static Quat SquadFullPath(const Quat& quat1, const Quat& tangent1, const Quat& quat2, const Quat& tangent2, f32 alpha)
		{
			const Quat q1 = SlerpFullPathNotNormalized(quat1, quat2, alpha);
			const Quat q2 = SlerpFullPathNotNormalized(tangent1, tangent2, alpha);
			return SlerpFullPath(q1, q2, 2.f * alpha * (1.f - alpha));
		}

		static void CalcTangents(const Quat& previous, const Quat& current, const Quat& next, f32 tension, Quat& outTangent)
		{
			const Quat inverse = current.Inverse();
			const Quat partA = (inverse * previous).Log();
			const Quat partB = (inverse * next).Log();
			const Quat preExp = (partA + partB) * -0.5f;
			outTangent = current * preExp.Exp();
		}

		static f32 NormalizeAxis(f32 degrees)
		{
			f32 angle = ClampAxis(degrees);

			if (angle > 180.f)
				angle -= 360.f;

			return angle;
		}

		static f32 ClampAxis(f32 degrees)
		{
			f32 angle = std::fmod(degrees, 360.f);

			if (angle < 0.f)
				angle += 360.f;

			return angle;
		}

		static f32 UnwindRadians(f32 radians)
		{
			f32 angle = radians;

			while (angle > Pi)
				angle -= 2.f * Pi;

			while (angle < -Pi)
				angle += 2.f * Pi;

			return angle;
		}

	private:
		static Quat FindBetweenHelper(const Vec3f& a, const Vec3f& b, f32 normAB)
		{
			f32 w = normAB + Vec3f::DotProduct(a, b);
			Quat result;

			if (w >= 1.e-6f * normAB)
			{
				result = Quat(
					a.Y() * b.Z() - a.Z() * b.Y(),
					a.Z() * b.X() - a.X() * b.Z(),
					a.X() * b.Y() - a.Y() * b.X(),
					w);
			}
			else
			{
				w = 0.f;

				if (Abs(a.X()) > Abs(a.Y()))
					result = Quat(-a.Z(), 0.f, a.X(), w);
				else
					result = Quat(0.f, -a.Z(), a.Y(), w);
			}

			result.Normalize();
			return result;
		}

		static f32 Abs(f32 value)
		{
			if (value < 0.f)
				return -value;

			return value;
		}

		static f32 Clamp(f32 value, f32 min, f32 max)
		{
			if (value < min)
				return min;

			if (value > max)
				return max;

			return value;
		}

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

	inline Quat operator*(f32 scalar, const Quat& value)
	{
		return value * scalar;
	}

	inline const Quat Quat::Identity = Quat(0.f, 0.f, 0.f, 1.f);
}
