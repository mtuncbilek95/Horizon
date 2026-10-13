#include "ScriptSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Game/GameScript.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Scripts/ScriptComponent.h>

namespace Horizon::Engine
{
	b8 ScriptSystem::OnInitialize()
	{
		return true;
	}

	void ScriptSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		if (!m_inSession)
			return;

		currentScene.ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
			{
				if (!comp.m_script)
					return;

				GameScript& script = *comp.m_script;

				if (!script.m_started)
					StartScript(script, handl, currentScene);

				script.OnUpdate(ctx.DeltaTime());
			});
	}

	void ScriptSystem::OnFinalize()
	{
		EndSession(GetWorldService()->GetCurrentWorld());
	}

	void ScriptSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
		Scene* pScene = GetWorldService()->GetCurrentWorld();

		if (previous == SceneGroups::Edit && current == SceneGroups::Play)
		{
			BeginSession();
			return;
		}

		if (previous == SceneGroups::Pause && current == SceneGroups::Edit)
		{
			EndSession(pScene);
			return;
		}

		if (previous == SceneGroups::Play && current == SceneGroups::Edit)
		{
			EndSession(pScene);
			return;
		}
	}

	void ScriptSystem::BeginSession()
	{
		m_inSession = true;
	}

	void ScriptSystem::EndSession(Scene* pScene)
	{
		if (!m_inSession)
			return;

		if (pScene != nullptr)
		{
			pScene->ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
				{
					if (!comp.m_script || !comp.m_script->m_started)
						return;

					comp.m_script->OnStop();
					comp.m_script->m_started = false;
					comp.m_script->m_scene = nullptr;
					comp.m_script->m_engine = nullptr;
				});
		}

		m_inSession = false;
	}

	void ScriptSystem::StartScript(GameScript& script, EntityHandle entity, Scene& scene)
	{
		script.m_entity = entity;
		script.m_scene = &scene;
		script.m_engine = GetEngine();
		script.m_started = true;

		script.OnStart();
	}
}