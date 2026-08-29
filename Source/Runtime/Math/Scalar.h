#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <cmath>

namespace Horizon::Math
{
	inline constexpr f32 Pi = 3.1415926535897932f;
	inline constexpr f32 TwoPi = 6.2831853071795865f;
	inline constexpr f32 HalfPi = 1.5707963267948966f;

	inline constexpr f32 SmallNumber = 1.e-8f;
	inline constexpr f32 KindaSmallNumber = 1.e-4f;
	inline constexpr f32 BigNumber = 3.4e+38f;

	template<typename T>
	constexpr T Abs(T value)
	{
		return value < T(0) ? -value : value;
	}

	template<typename T>
	constexpr T Min(T lhs, T rhs)
	{
		return lhs < rhs ? lhs : rhs;
	}

	template<typename T>
	constexpr T Max(T lhs, T rhs)
	{
		return lhs > rhs ? lhs : rhs;
	}

	template<typename T>
	constexpr T Clamp(T value, T low, T high)
	{
		if (value < low)
			return low;

		if (value > high)
			return high;

		return value;
	}

	template<typename T>
	constexpr T Sign(T value)
	{
		if (value < T(0))
			return T(-1);

		if (value > T(0))
			return T(1);

		return T(0);
	}

	template<typename T>
	constexpr T Square(T value)
	{
		return value * value;
	}

	constexpr f32 Saturate(f32 value)
	{
		return Clamp(value, 0.f, 1.f);
	}

	constexpr f32 Lerp(f32 from, f32 to, f32 alpha)
	{
		return from + (to - from) * alpha;
	}

	constexpr f32 InverseLerp(f32 from, f32 to, f32 value)
	{
		const f32 range = to - from;

		if (Abs(range) <= SmallNumber)
			return 0.f;

		return (value - from) / range;
	}

	constexpr f32 SmoothStep(f32 from, f32 to, f32 value)
	{
		const f32 alpha = Saturate(InverseLerp(from, to, value));
		return alpha * alpha * (3.f - 2.f * alpha);
	}

	constexpr b8 IsNearlyEqual(f32 lhs, f32 rhs, f32 tolerance = KindaSmallNumber)
	{
		return Abs(lhs - rhs) <= tolerance;
	}

	constexpr b8 IsNearlyZero(f32 value, f32 tolerance = KindaSmallNumber)
	{
		return Abs(value) <= tolerance;
	}

	constexpr f32 DegToRad(f32 degrees)
	{
		return degrees * (Pi / 180.f);
	}

	constexpr f32 RadToDeg(f32 radians)
	{
		return radians * (180.f / Pi);
	}

	constexpr f32 MoveTowards(f32 current, f32 target, f32 maxDelta)
	{
		const f32 delta = target - current;

		if (Abs(delta) <= maxDelta)
			return target;

		return current + Sign(delta) * maxDelta;
	}

	template<typename T>
	constexpr T FloorDiv(T value, T divisor)
	{
		const T quotient = value / divisor;

		if ((value % divisor != T(0)) && ((value < T(0)) != (divisor < T(0))))
			return quotient - T(1);

		return quotient;
	}

	template<typename T>
	constexpr T FloorMod(T value, T divisor)
	{
		const T remainder = value % divisor;

		if (remainder != T(0) && ((remainder < T(0)) != (divisor < T(0))))
			return remainder + divisor;

		return remainder;
	}

	inline f32 WrapAngle(f32 radians)
	{
		f32 wrapped = std::fmod(radians + Pi, TwoPi);

		if (wrapped < 0.f)
			wrapped += TwoPi;

		return wrapped - Pi;
	}

	inline f32 ShortestAngleDelta(f32 from, f32 to)
	{
		return WrapAngle(to - from);
	}

	inline f32 SnapToGrid(f32 value, f32 gridSize)
	{
		if (IsNearlyZero(gridSize, SmallNumber))
			return value;

		return std::floor((value / gridSize) + 0.5f) * gridSize;
	}
}