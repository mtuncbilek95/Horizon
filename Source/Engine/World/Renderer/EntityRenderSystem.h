#pragma once

#include <Engine/World/Core/EntitySystem.h>

namespace Horizon
{
	class GfxDevice;
	class GfxCommandPool;
	class GfxCommandBuffer;
	class GfxQueue;

	class Engine;
	class FrameGraph;

	class EntityRenderSystem : public EntitySystem<EntityRenderSystem>
	{
	public:
		EntityRenderSystem();
		~EntityRenderSystem();

		void OnTick() final;

	private:
		b8 OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		GfxDevice* m_device;
		GfxQueue* m_gQueue;

		std::unique_ptr<FrameGraph> m_graph;
		std::shared_ptr<GfxCommandPool> m_pool;
		std::vector<std::shared_ptr<GfxCommandBuffer>> m_cmdBuffers;
	};
}