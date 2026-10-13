#pragma once

#include <Engine/World/ECS/Definitions.h>
#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class GameScript;

	HCLASS(SystemOrder[SystemPhase::Script]);
	class ENGINE_API ScriptSystem : public System
	{
		HORIZON_TYPE_REFLECT(ScriptSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::Script; }

	private:
		void BeginSession();
		void EndSession(Scene* pScene);
		void StartScript(GameScript& script, EntityHandle entity, Scene& scene);

	private:
		b8 m_inSession = false;
	};
}