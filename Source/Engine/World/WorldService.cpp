#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/System.h>

#include <Runtime/Containers/ScopedLock.h>

// TODO: Remove this later
#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/NameComponent.h>
#include <Engine/Asset/Asset.h>
#include <Engine/Asset/AssetLoadStrategy.h>
#include <Engine/Asset/Scene/SceneInstantiator.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		// Try to catch all the systems first.
		ReflectionSystem* pReflection = GetEngine()->GetReflectionSystem();

		List<Reflect::Type*> systemTypes = pReflection->GetTypeByBase(Reflect::TypeOf<System>());
		for (auto* pType : systemTypes)
		{
			auto* pSystem = (System*)pType->Create(Memory::CurrLoc());
			pSystem->m_engine = GetEngine();
			if (pSystem->OnInitialize())
			{
				Terminal::Info(StringOps::GetName(this), "{} has been initialized properly.", pType->GetName());
				m_systems.PushBack(pSystem);
			}
			else
				Terminal::Error(StringOps::GetName(this), "{} has failed initializing.", pType->GetName());
		}

		// TODO: THIS WHOLE THING IS TEMPORARY
		m_activeWorld = Memory::Allocator::Create<World>(Memory::CurrLoc(), pReflection);

		if (!m_activeWorld)
			return ModuleReport();

		const std::string scenePath = "D:/Projects/Horizon/ExampleProject/Cooked/TestWorld.hfile";

		PAL::FileAccessRequest request = PAL::File::RequestAccess(scenePath,
			PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		List<u8> payload;
		const b8 wasRead = PAL::File::ReadMemory(request, payload);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "'{}' could not be read", scenePath);
			return ModuleReport();
		}

		AssetLoadStrategy* pStrategy = GetEngine()->RequestService<AssetService>()->FindStrategy(Reflect::TypeOf<SceneAsset>());

		if (!pStrategy)
			return ModuleReport();

		Asset* pAsset = pStrategy->Create(std::move(payload));

		if (!pAsset)
			return ModuleReport();

		if (!SceneInstantiator::Apply(*static_cast<SceneAsset*>(pAsset), *m_activeWorld, pReflection))
			Terminal::Error(StringOps::GetName(this), "'{}' could not be applied to the world", scenePath);

		pStrategy->Destroy(pAsset);

		return ModuleReport();
	}

	void WorldService::OnExecute()
	{
		// TODO: Is this okay?
		for (auto* pSys : m_systems)
			pSys->OnExecute(*m_activeWorld);
	}

	void WorldService::OnFinalize()
	{
		for (auto* pSys : m_systems)
		{
			pSys->OnFinalize();
			Memory::Allocator::Delete(pSys);
		}

		Memory::Allocator::Delete(m_activeWorld);
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
		graph.Requires<AssetService>();
	}
}