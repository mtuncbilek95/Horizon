#pragma once

#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Engine/World/System.h>
#include <Engine/World/Systems/Rendering/RenderSlot.h>
#include <Engine/Rendering/RenderGraph/RenderPass.h>

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/Math/Vec2u.h>

namespace Horizon::Engine
{
	class MeshAssetStreamer;

	HCLASS(SystemOrder[u32_max]);
	class ENGINE_API RenderSystem : public System
	{
		HORIZON_TYPE_REFLECT(RenderSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;

		u64 GetOutputView() const;

		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;
		SystemGroup GetWorkingGroup() const final { return SystemGroup::Render; }

		const Math::Vec2u& GetImageSize() const { return m_targetSize; }
		void ResizeImage(const Math::Vec2u& imgSize);

	private:
		void CreateSlot(usize index);
		void DestroySlot(usize index);

	private:
		GraphicsContext* m_context = nullptr;
		RHI::GfxDevice* m_device = nullptr;
		RHI::GfxQueue* m_queue = nullptr;

		Math::Vec2u m_targetSize = { 0, 0 };

		RHI::GfxDescriptorHeap* m_colorTargetHeap = nullptr;
		RHI::GfxDescriptorHeap* m_depthTargetHeap = nullptr;
		RHI::GfxDescriptorHeap* m_resourceHeap = nullptr;

		List<RenderSlot> m_slots;
		List<RenderPass*> m_renderPasses;

		RHI::GfxFence* m_fence = nullptr;
		u32 m_frameIndex = 0;

		RHI::GfxBuffer* m_frameDataBuffer = nullptr;
		u8* m_frameDataMap = nullptr;

		u32 m_frameDataStride = 0;
		u32 m_frameDataViews[GraphicsContext::MaxFramesInFlight] = {};
	};
}