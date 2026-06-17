#include "GraphicsModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/ECS/EntityComponentModule.h>

namespace Horizon
{
	void GraphicsModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto& windowModule = engine.GetModule<WindowModule>();
		const Window& window = windowModule.GetMainWindow();

		m_device = CreateGfxDevice();

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);
	}

	void GraphicsModule::OnSync()
	{
		auto& ecsModule = m_engine->GetModule<EntityComponentModule>();
	}

	void GraphicsModule::OnDetach()
	{
	}
}
