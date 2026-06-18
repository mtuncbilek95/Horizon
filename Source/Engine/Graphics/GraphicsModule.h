#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/Graphics/FrameContext.h>

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandList.h>
#include <Runtime/Graphics/RHI/Texture/GfxTexture.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>

namespace Horizon
{
	class GraphicsModule final : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

		GfxDevice* GetDevice() const { return &*m_device; }
		GfxQueue* GetGraphicsQueue() const { return &*m_graphicsQueue; }
		GfxQueue* GetComputeQueue() const { return &*m_graphicsQueue; }
		GfxQueue* GetTransferQueue() const { return &*m_graphicsQueue; }

		FrameContext& GetFrameContext() { return m_frameContext; }

		GfxTexture* GetSceneColor() const { return m_sceneColor.GetRawPtr(); }
		u64 GetSceneTextureId() const { return m_device->GetImGuiTextureId(m_sceneColor.GetRawPtr()); }

	private:
		std::unique_ptr<GfxDevice> m_device;

		std::unique_ptr<GfxQueue> m_graphicsQueue;
		std::unique_ptr<GfxQueue> m_computeQueue;
		std::unique_ptr<GfxQueue> m_transferQueue;

		FrameContext m_frameContext;
		std::unique_ptr<GfxCommandList> m_sceneCommandList;

		GfxPointer<GfxTexture> m_sceneColor;
		u32 m_sceneWidth = 0;
		u32 m_sceneHeight = 0;
	};
}
