#include "JoltDebugRenderer.h"

namespace Horizon::Engine
{
	void JoltDebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color)
	{
		if (!m_target)
			return;

		const u32 packed = color.GetUInt32();

		m_target->PushBack({ Math::Vec3f(from.GetX(), from.GetY(), from.GetZ()), packed });
		m_target->PushBack({ Math::Vec3f(to.GetX(), to.GetY(), to.GetZ()), packed });
	}

	void JoltDebugRenderer::DrawText3D(JPH::RVec3Arg position, const std::string_view& text, JPH::ColorArg color, f32 height)
	{
	}
}