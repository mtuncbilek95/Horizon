#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Presentation/PresentationOverlay.h>

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Command/CommandPool.h>
#include <Runtime/RHI/Fence/Fence.h>
#include <Runtime/RHI/Swapchain/Swapchain.h>

namespace Horizon::PAL
{
	class Window;
}

namespace Horizon::RHI
{
	class Device;
	class DescriptorHeap;
}

namespace Horizon::Engine
{
	enum class PresentationMode : u8
	{
		Game,
		Editor
	};

	class PresentationService final : public Service
	{
		struct FrameSlot
		{
			UniquePtr<RHI::CommandPool> pPool;
			UniquePtr<RHI::CommandList> pCmdList;
			u64 fenceValue = 0;
		};

	public:
		static constexpr RHI::TextureFormat SceneFormat = RHI::TextureFormat::RGBA16_FLOAT;

	public:
		explicit PresentationService(PresentationMode mode);
		~PresentationService();

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

		PresentationMode GetMode() const { return m_mode; }
		void SetOverlay(PresentationOverlay* pOverlay) { m_overlay = pOverlay; }

		RHI::Swapchain* GetSwapchain() const { return m_swapchain.Get(); }

		void RequestSceneExtent(Math::Vec2u extent) { m_pendingExtent = extent; }
		Math::Vec2u GetSceneExtent() const { return m_sceneExtent; }
		RHI::Texture* GetSceneTexture() const { return m_sceneTexture.Get(); }
		RHI::TextureView* GetSceneView() const { return m_sceneView.Get(); }

	private:
		void HandleWindowMessages();
		void Record(RHI::CommandList* pCmdList);
		b8 CreateSceneTarget(Math::Vec2u extent);
		void ApplySceneExtent();

	private:
		PresentationMode m_mode = PresentationMode::Game;
		PresentationOverlay* m_overlay = nullptr;

		PAL::Window* m_window = nullptr;
		RHI::Device* m_device = nullptr;
		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		RHI::DescriptorHeap* m_samplerHeap = nullptr;

		UniquePtr<RHI::Swapchain> m_swapchain;
		b8 m_minimized = false;

		List<FrameSlot> m_frames;
		UniquePtr<RHI::Fence> m_fence;
		u32 m_frameIndex = 0;

		UniquePtr<RHI::Texture> m_sceneTexture;
		UniquePtr<RHI::TextureView> m_sceneView;
		Math::Vec2u m_sceneExtent;
		Math::Vec2u m_pendingExtent;
	};
}