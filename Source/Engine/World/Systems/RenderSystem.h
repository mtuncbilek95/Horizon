#pragma once

#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Engine/World/System.h>
#include <Engine/World/Systems/RenderSlot.h>

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/Math/Vec2u.h>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[u32_max]);
	class H_EXPORT RenderSystem : public System
	{
		HORIZON_TYPE_REFLECT(RenderSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;

		u64 GetSceneView() const;

		const Math::Vec2u& GetImageSize() const { return m_targetSize; }
		void ResizeImage(const Math::Vec2u& imgSize);

	private:
		b8 RecreateSlot(u32 imageIndex);
		b8 ClearSlot(u32 imageIndex);

	private:
		GraphicsContext* m_context = nullptr;
		RHI::GfxDevice* m_device = nullptr;
		RHI::GfxQueue* m_queue = nullptr;

		RHI::GfxDescriptorHeap* m_resourceHeap = nullptr;
		RHI::GfxDescriptorHeap* m_colorHeap = nullptr;

		List<RenderSlot> m_slots;

		Math::Vec2u m_targetSize = { 0, 0 };

		RHI::GfxFence* m_fence = nullptr;
		u32 m_frameIndex = 0;
	};
}