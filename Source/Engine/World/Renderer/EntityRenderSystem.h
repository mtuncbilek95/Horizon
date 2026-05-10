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

		void OnInitialize();
		void OnTick() final;

	private:
		FrameGraph* m_graph;
		GfxDevice* m_device;
		GfxQueue* m_gQueue;

		std::shared_ptr<GfxCommandPool> m_pool;
		std::vector<std::shared_ptr<GfxCommandBuffer>> m_cmdBuffers;
	};
}