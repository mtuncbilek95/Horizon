#include "TransformSystem.h"

#include <Engine/World/Components/TransformComponent.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	b8 TransformSystem::OnInitialize()
	{
		return true;
	}

	void TransformSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		currentScene.ForEach<TransformComponent>([&](EntityHandle handl, TransformComponent& transform)
			{
				if (transform.m_rotation != transform.m_lastRotation)
					transform.m_orientation = Math::Quat::FromEuler(transform.m_rotation);
				else
				{
					const Math::Quat fromEuler = Math::Quat::FromEuler(transform.m_rotation);
					const f32 alignment = Math::Abs(fromEuler | transform.m_orientation);

					if (1.f - alignment > Math::Quat::KindaSmallNumber)
						transform.m_rotation = transform.m_orientation.ToEuler();
				}

				transform.m_lastRotation = transform.m_rotation;
				transform.m_worldMatrix = Math::Mat4f::TRS(transform.m_position, transform.m_orientation, transform.m_scale);
			});
	}

	void TransformSystem::OnFinalize()
	{
	}
}