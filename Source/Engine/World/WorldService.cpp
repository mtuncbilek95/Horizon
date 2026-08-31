#include "WorldService.h"

#include <Engine/Asset/Asset.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/AssetLoadStrategy.h>
#include <Engine/Asset/Scene/SceneAsset.h>
#include <Engine/Asset/Scene/SceneChunk.h>
#include <Engine/Asset/Scene/SceneInstantiator.h>
#include <Engine/Content/ContentContext.h>
#include <Engine/Content/ContentFileReader.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/System.h>

#include <Runtime/Containers/ScopedLock.h>

#include <utility>

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

		m_activeWorld = Memory::Allocator::Create<World>(Memory::CurrLoc(), pReflection);

		if (m_activeWorld == nullptr)
			return ModuleReport("Active world could not be created");

		return ModuleReport();
	}

	void WorldService::OnExecute()
	{
		if (m_activeWorld == nullptr)
			return;

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

		m_systems.Clear();

		Memory::Allocator::Delete(m_activeWorld);
		m_activeWorld = nullptr;
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
		graph.Requires<ContentContext>();
		graph.Requires<AssetService>();
	}

	b8 WorldService::LoadScene(const Guid& sceneId)
	{
		if (!sceneId.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "An invalid scene id cannot be loaded");
			return false;
		}

		if (m_activeWorld == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "There is no active world to load into");
			return false;
		}

		ContentContext* pContent = GetEngine()->RequestContext<ContentContext>();

		if (pContent == nullptr)
			return false;

		List<u8> file;

		if (!pContent->Read(sceneId, file))
			return false;

		ContentFileReader reader(file.GetData(), file.GetCount());

		if (!reader.IsValid())
			return false;

		if (reader.GetAssetTypeName() != "SceneAsset")
		{
			Terminal::Error(StringOps::GetName(this), "{} carries a {}, not a scene", sceneId.ToString(),
				reader.GetAssetTypeName());
			return false;
		}

		List<u8> payload;

		if (!reader.ReadSection(SceneChunkSectionId, payload))
			return false;

		AssetLoadStrategy* pStrategy = GetEngine()->RequestService<AssetService>()->FindStrategy(Reflect::TypeOf<SceneAsset>());

		if (pStrategy == nullptr)
			return false;

		Asset* pAsset = pStrategy->Create(std::move(payload));

		if (pAsset == nullptr)
			return false;

		ReflectionSystem* pReflection = GetEngine()->GetReflectionSystem();

		m_activeWorld->Clear();

		const b8 applied = SceneInstantiator::Apply(*static_cast<SceneAsset*>(pAsset), *m_activeWorld, pReflection);

		pStrategy->Destroy(pAsset);

		if (!applied)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be applied, the world is left empty",
				sceneId.ToString());
			return false;
		}

		m_activeSceneId = sceneId;

		Terminal::Info(StringOps::GetName(this), "{} has been loaded into the active world", sceneId.ToString());

		return true;
	}
}