#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/System.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();
		m_reflection = pReflect;

		List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<System>());
		for (auto* pType : types)
		{
			auto* pSystem = (System*)pType->Create();
			if (!pSystem)
			{
				Terminal::Error(StringOps::GetName(this), "{} has no default constructor. You won't have this system.", pType->GetName());
				continue;
			}
			pSystem->m_engine = GetEngine();

			if (pSystem->OnInitialize())
			{
				Terminal::Info(StringOps::GetName(this), "{} has been registered to WorldService.", pType->GetName());
				m_systemLookup[pType->GetTypeId()] = m_systems.GetCount();
				m_systems.PushBack(pSystem);
			}
		}

		return ModuleReport();
	}

	void WorldService::OnExecute(const EngineFrame& ctx)
	{
		for (auto* pSystem : m_systems)
			pSystem->OnExecute(ctx);
	}

	void WorldService::OnFinalize()
	{
		for (auto* pSystem : m_systems)
		{
			pSystem->OnFinalize();
			Reflect::Type* pType = m_reflection->GetType(pSystem->GetTypeId());
			pType->Destroy(pSystem);
		}
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<AssetService>();
		graph.Requires<GraphicsContext>();
	}

	System* WorldService::RequestSystem(Reflect::TypeHandle handl) const
	{
		auto it = m_systemLookup.find(handl);
		if (it == m_systemLookup.end())
			return nullptr;

		return m_systems[it->second];
	}
}