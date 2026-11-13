#pragma once

#include <Engine/Rendering/Renderables/RenderablePhysicsDebug.h>

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

namespace Horizon::Engine
{
	class JoltDebugRenderer final : public JPH::DebugRendererSimple
	{
	public:
		void SetTarget(List<DebugLineVertex>* pLines) { m_target = pLines; }

		void DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color) final;
		void DrawText3D(JPH::RVec3Arg position, const std::string_view& text, JPH::ColorArg color, f32 height) final;

	private:
		List<DebugLineVertex>* m_target = nullptr;
	};
}