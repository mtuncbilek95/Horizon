#pragma once

#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Engine/World/System.h>

#include <Runtime/RHI/Texture/GfxTexture.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/Math/Vec2u.h>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[300]);
	class H_EXPORT RenderSystem : public System
	{
		HORIZON_TYPE_REFLECT(RenderSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;

		u64 GetSceneView() const;

		const Math::Vec2u& GetImageSize() { return m_workableArea; }
		void ResizeImage(const Math::Vec2u& imgSize);

	private:
		b8 CreateTexture(const Math::Vec2u& imgSize);
		void DestroyTexture();

		b8 EnsureTargets();
		RHI::GfxCommandList* BeginFrame();
		void BuildFrameData(const EngineFrame& ctx, Scene& currentScene);
		void RenderScene(RHI::GfxCommandList* pCommand, RHI::GfxTexture* pTarget);
		void EndFrame(RHI::GfxCommandList* pCommand);

	private:
		GraphicsContext* m_context = nullptr;
		RHI::GfxDevice* m_device = nullptr;
		RHI::GfxQueue* m_queue = nullptr;

		RHI::GfxDescriptorHeap* m_resourceHeap = nullptr;
		RHI::GfxDescriptorHeap* m_colorHeap = nullptr;

		Math::Vec2u m_workableArea;
		RHI::GfxTexture* m_lastImage = nullptr;
		RHI::GfxResourceState m_imageState = RHI::GfxResourceState::Common;

		List<RHI::GfxCommandList*> m_commandLists;
		List<u64> m_frameValues;
		RHI::GfxFence* m_fence = nullptr;
		u32 m_frameIndex = 0;
	};
}