#pragma once

#include <Engine/Core/Context.h>

namespace Horizon
{
	class GfxInstance;
	class GfxDevice;
	class GfxQueue;
	class GfxQueue;
	class GfxQueue;

	class GfxContext : public Context<GfxContext>
	{
	public:
		GfxInstance& Instance() const { return *m_instance.get(); }
		GfxDevice& Device() const { return *m_device.get(); }
		GfxQueue& GraphicsQueue() const { return *m_graphicsQueue.get(); }
		GfxQueue& ComputeQueue() const { return *m_computeQueue.get(); }
		GfxQueue& TransferQueue() const { return *m_transferQueue.get(); }

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		std::shared_ptr<GfxInstance> m_instance;
		std::shared_ptr<GfxDevice> m_device;

		std::shared_ptr<GfxQueue> m_graphicsQueue;
		std::shared_ptr<GfxQueue> m_computeQueue;
		std::shared_ptr<GfxQueue> m_transferQueue;
	};
}