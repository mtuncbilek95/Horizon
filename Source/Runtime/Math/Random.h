#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Math
{
	class Random final
	{
	public:

	private:
		u64 m_state;
		u64 m_increment;
	};
}