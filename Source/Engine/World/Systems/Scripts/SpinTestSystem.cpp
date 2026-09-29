#include "SpinTestSystem.h"

#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/Components/Scripts/SpinTestComponent.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	b8 SpinTestSystem::OnInitialize()
	{
		return true;
	}

	void SpinTestSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		currentScene.ForEach<TransformComponent, SpinTestComponent>([&](EntityHandle handl, TransformComponent& transform,
			SpinTestComponent& spin)
			{
				const f32 lengthSq = spin.m_axis | spin.m_axis;
				if (lengthSq < Math::Quat::SmallNumber)
					return;

				const Math::Vec3f axis = spin.m_axis / Math::Sqrt(lengthSq);
				const f32 angle = Math::DegToRad(spin.m_speed) * ctx.DeltaTime();
				const Math::Quat delta = Math::Quat::FromAxisAngle(axis, angle);

				transform.m_orientation = delta * transform.m_orientation;
				transform.m_orientation.Normalize();
			});
	}

	void SpinTestSystem::OnFinalize()
	{
	}

	void SpinTestSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{

	}

}