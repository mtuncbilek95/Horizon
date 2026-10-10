#include "ScriptSystem.h"

#include <Engine/Core/Engine.h>
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
		if (!m_worldService)
			m_worldService = GetEngine()->RequestService<WorldService>();

		// Just in case
		if (!HasFlag(m_worldService->GetRunningSystems(), SystemGroup::Script))
			return;

		currentScene.ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
			{
				// TODO: Should do only OnUpdate(ctx.DeltaTime())? Or we need more stuff here?
			});
	}

	void ScriptSystem::OnFinalize()
	{
		Scene* pScene = GetWorldService()->GetCurrentWorld();
		if (!pScene)
			return;

		pScene->ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
			{
				// TODO: OnStop();
			});
	}

	void ScriptSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
		Scene* pScene = GetWorldService()->GetCurrentWorld();
		if (!pScene)
			return;

		// Stop to play
		if (previous == SceneGroups::Edit && current == SceneGroups::Play)
		{
			pScene->ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
				{
					// TODO: OnStart();
				});
			return;
		}

		// Pause to stop
		if (previous == SceneGroups::Pause && current == SceneGroups::Edit)
		{
			pScene->ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
				{
					// TODO: OnStop();
				});
			return;
		}

		// Play to stop
		if (previous == SceneGroups::Edit && current == SceneGroups::Play)
		{
			pScene->ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
				{
					// TODO: OnStop();
				});
			return;
		}
	}
}