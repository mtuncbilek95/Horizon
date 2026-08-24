#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>

// TODO: Remove this later
#include <Engine/World/Components/TransformComponent.h>

#include <Runtime/Containers/ScopedLock.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		// TODO: This is not correct tho. I will change it to asset + default scene/world.
		m_activeWorld = Memory::Allocator::Create<World>(Memory::CurrLoc());

		// TODO: Remove this later to test properly.
		EntityHandle e1 = m_activeWorld->CreateEntity();
		EntityHandle e2 = m_activeWorld->CreateEntity();
		EntityHandle e3 = m_activeWorld->CreateEntity();
		EntityHandle e4 = m_activeWorld->CreateEntity();
		EntityHandle e5 = m_activeWorld->CreateEntity();
		EntityHandle e6 = m_activeWorld->CreateEntity();

		m_activeWorld->AddComponent(e1, TransformComponent());
		m_activeWorld->AddComponent(e2, TransformComponent());
		m_activeWorld->AddComponent(e3, TransformComponent());
		m_activeWorld->AddComponent(e4, TransformComponent());

		if (!m_activeWorld)
			return ModuleReport();

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