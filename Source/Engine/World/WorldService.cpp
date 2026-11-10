#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/System.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		m_graph = MakeUnique<SystemGraph>();
		m_graph->m_engine = GetEngine();

		if (m_graph->OnInititalize())
			Terminal::Info(StringHelper::GetTypeName(this), "SystemGraph has been initialized successfully");

		return ModuleReport();
	}

	void WorldService::OnExecute(const EngineFrame& ctx)
	{
		m_blackboard.Reset();

		if (!m_activeWorld)
			return;

		m_graph->OnExecute(ctx, m_activeWorld);
	}

	void WorldService::OnFinalize()
	{
		m_graph->OnFinalize();
	}

	void WorldService::DeclareDependencies(ModuleDependency& graph)
	{
		graph.InitAfter<AssetService, GraphicsContext>();
	}

	System* WorldService::FindSystem(Reflect::TypeHandle handl)
	{
		return m_graph->FindSystem(handl);
	}
}