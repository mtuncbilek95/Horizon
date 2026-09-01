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

		List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<System>());
		for (auto* pType : types)
		{
			auto* pSystem = (System*)pType->Create();
			pSystem->m_engine = GetEngine();

			if (pSystem->OnInitialize())
			{
				Terminal::Info(StringOps::GetName(this), "{} has been registered to WorldService.", pType->GetName());
				m_systems.PushBack(pSystem);
			}
		}

		return ModuleReport();
	}

	void WorldService::OnExecute()
	{
		for (auto* pSystem : m_systems)
			pSystem->OnExecute();
	}

	void WorldService::OnFinalize()
	{
		for (auto* pSystem : m_systems)
		{
			pSystem->OnFinalize();
			Memory::Allocator::Delete(pSystem);
		}
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<AssetService>();
		graph.Requires<GraphicsContext>();
	}
}