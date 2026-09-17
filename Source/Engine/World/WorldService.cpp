#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Engine/World/ECS/Scene.h>

#include <Engine/World/Components/NameComponent.h>
#include <Engine/World/Components/CameraComponent.h>
#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/MeshComponent.h>

#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/CameraComponent.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();
		m_reflection = pReflect;

		List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<System>());
		List<SystemEntry> entries;
		for (auto* pType : types)
		{
			auto* pAttr = pType->GetCustomAttribute<SystemOrderAttribute>();
			if (!pAttr)
			{
				Terminal::Error(StringOps::GetName(this), "{} has no SystemOrderAttribute. You won't have this system.", pType->GetName());
				continue;
			}

			auto* pSystem = (System*)pType->Create();
			if (!pSystem)
			{
				Terminal::Error(StringOps::GetName(this), "{} has no default constructor. You won't have this system.", pType->GetName());
				continue;
			}

			pSystem->m_engine = GetEngine();
			pSystem->m_ownerService = this;

			entries.EmplaceBack(pSystem, pType->GetName(), pAttr->GetOrderNumber());
		}

		entries.Sort([&](const SystemEntry& a, const SystemEntry& b)
			{
				return a.order < b.order;
			});

		for (const auto& entry : entries)
		{
			if (entry.pSystem->OnInitialize())
			{
				Terminal::Info(StringOps::GetName(this), "{} has been registered to WorldService.", entry.name);
				m_systemLookup[entry.pSystem->GetTypeId()] = m_systems.GetCount();
				m_systems.PushBack(entry.pSystem);
			}
		}

		m_activeWorld = Memory::Allocator::Create<Scene>(Memory::CurrLoc());

		{
			EntityHandle e1 = m_activeWorld->AddEntity();
			auto* nComp = m_activeWorld->AddComponent(e1, NameComponent());
			auto* tComp = m_activeWorld->AddComponent(e1, TransformComponent());
			auto* mComp = m_activeWorld->AddComponent(e1, MeshComponent());
			nComp->m_name = NameId("SquareObject");
		}

		{
			EntityHandle e1 = m_activeWorld->AddEntity();
			auto* nComp = m_activeWorld->AddComponent(e1, NameComponent());
			auto* tComp = m_activeWorld->AddComponent(e1, TransformComponent());
			auto* cComp = m_activeWorld->AddComponent(e1, CameraComponent());
			nComp->m_name = NameId("Perspective Camera");
		}

		return ModuleReport();
	}

	void WorldService::OnExecute(const EngineFrame& ctx)
	{
		for (auto* pSystem : m_systems)
			pSystem->OnExecute(ctx, *m_activeWorld);
	}

	void WorldService::OnFinalize()
	{
		for (auto* pSystem : m_systems)
		{
			pSystem->OnFinalize();
			Reflect::Type* pType = m_reflection->GetType(pSystem->GetTypeId());
			pType->Destroy(pSystem);
		}

		Memory::Allocator::Delete(m_activeWorld);
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