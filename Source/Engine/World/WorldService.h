#pragma once

#include <Engine/Core/Service.h>
#include <Engine/World/World.h>
#include <Engine/World/WorldCommandBuffer.h>

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Mutex.h>

namespace Horizon::Engine
{
	class H_EXPORT WorldService : public Service
	{
	public:
		WorldService() = default;
		~WorldService() = default;

		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		World* GetActiveWorld() const { return m_activeWorld; }
		WorldCommandBuffer& GetCommandBuffer();

	private:
		void FlushCommandBuffers();

	private:
		World* m_activeWorld = nullptr;
		List<WorldCommandBuffer*> m_commandBuffers;
		PAL::Mutex m_bufferGuard;
	};
}