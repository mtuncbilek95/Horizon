#pragma once

#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[SystemPhase::Camera]);
	class ENGINE_API CameraSystem : public System
	{
		HORIZON_TYPE_REFLECT(CameraSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::General; }

		void SetOverride(EntityHandle camera) { m_override = camera; }
		void ClearOverride() { m_override = EntityHandle(); }
		EntityHandle GetOverride() const { return m_override; }
		EntityHandle GetActiveCamera() const { return m_active; }

	private:
		EntityHandle ResolveActiveCamera(Scene& currentScene) const;

	private:
		EntityHandle m_override;
		EntityHandle m_active;
	};
}