#include "GfxContext.h"

#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>

#include <Engine/Cache/CacheContext.h>

namespace Horizon
{
	EngineReport GfxContext::OnInitialize()
	{
		auto& cacheCtx = RequestContext<CacheContext>();
		auto& gfxCache = cacheCtx.GetGfxCache();

		m_instance = GfxInstance::Create(GfxInstanceDesc()
			.setAPIType(gfxCache.rhiType)
			.setAppName(gfxCache.appName)
			.setAppVersion(gfxCache.appVersion));
		if (!m_instance)
			return EngineReport("RHI", "failed initializing GfxInstance");

		m_device = m_instance->CreateDevice(GfxDeviceDesc()
			.setComputeQueueCount(1)
			.setGraphicsQueueCount(1)
			.setTransferQueueCount(1));
		if (!m_device)
			return EngineReport("RHI", "failed initializing GfxDevice");

		m_graphicsQueue = m_device->CreateQueue(QueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(QueueType::Compute);
		m_transferQueue = m_device->CreateQueue(QueueType::Transfer);

		return EngineReport();
	}

	void GfxContext::OnFinalize()
	{
		auto& cacheCtx = RequestContext<CacheContext>();

		auto& gfxCache = cacheCtx.GetGfxCache();
		gfxCache.rhiType = m_instance->Backend();
		gfxCache.appName = m_instance->AppName();
	}
}