#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
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
		// TODO: THIS WHOLE THING IS TEMPORARY
		ReflectionSystem* pReflection = GetEngine()->GetReflectionSystem();

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
		m_activeWorld->EndStructuralPhase();

		// TODO: Systems run here, on the job system, against a frozen layout

		m_activeWorld->BeginStructuralPhase();
		FlushCommandBuffers();
	}

	void WorldService::OnFinalize()
	{
		for (WorldCommandBuffer* pBuffer : m_commandBuffers)
			Memory::Allocator::Delete(pBuffer);

		Memory::Allocator::Delete(m_activeWorld);
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
		graph.Requires<AssetService>();
	}

	WorldCommandBuffer& WorldService::GetCommandBuffer()
	{
		thread_local WorldCommandBuffer* pLocal = nullptr;

		if (pLocal)
			return *pLocal;

		ScopedLock lock(m_bufferGuard);
		pLocal = Memory::Allocator::Create<WorldCommandBuffer>(Memory::CurrLoc());
		m_commandBuffers.PushBack(pLocal);

		return *pLocal;
	}

	void WorldService::FlushCommandBuffers()
	{
		List<WorldCommandBuffer::Command*> merged;

		for (WorldCommandBuffer* pBuffer : m_commandBuffers)
		{
			for (WorldCommandBuffer::Command& command : pBuffer->m_commands)
				merged.PushBack(&command);
		}

		merged.Sort([](WorldCommandBuffer::Command* pLeft, WorldCommandBuffer::Command* pRight)
			{
				return pLeft->sequence < pRight->sequence;
			});

		for (WorldCommandBuffer::Command* pCommand : merged)
			pCommand->action(*m_activeWorld);

		for (WorldCommandBuffer* pBuffer : m_commandBuffers)
			pBuffer->Clear();
	}
}