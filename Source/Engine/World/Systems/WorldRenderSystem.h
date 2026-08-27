#pragma once

#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/System.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Texture/GfxTexture.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT WorldRenderSystem : public System
	{
		HORIZON_TYPE_REFLECT(WorldRenderSystem);
	public:
		WorldRenderSystem() = default;
		~WorldRenderSystem() = default;

		b8 OnInitialize() final;
		void OnExecute(World& world) final;
		void OnFinalize() final;

		void RequestSize(u32 width, u32 height);

		RHI::GfxTexture* GetColorTarget() const { return m_colorTarget; }
		u64 GetColorTargetHandle() const;

	private:
		b8 CreateColorTarget(u32 width, u32 height);
		void DestroyColorTarget();

	private:
		GraphicsContext* m_graphics = nullptr;

		RHI::GfxDescriptorHeap* m_resourceHeap = nullptr;
		RHI::GfxDescriptorHeap* m_colorHeap = nullptr;

		RHI::GfxTexture* m_colorTarget = nullptr;
		RHI::GfxResourceState m_colorState = RHI::GfxResourceState::Common;

		RHI::GfxFence* m_fence = nullptr;

		List<RHI::GfxCommandList*> m_commandLists;
		List<u64> m_frameValues;

		u32 m_frameIndex = 0;

		u32 m_targetWidth = 0;
		u32 m_targetHeight = 0;
		u32 m_requestedWidth = 0;
		u32 m_requestedHeight = 0;
	};
}