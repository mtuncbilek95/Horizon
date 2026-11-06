#pragma once

#include <Engine/Presentation/PresentationOverlay.h>

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::RHI
{
	class Device;
	class DescriptorHeap;
	class CommandList;
}

namespace Horizon::Engine
{
	class WindowService;
}

namespace Horizon::Editor
{
	class EditorPlatform;

	using RenderContext = void*;

	struct EditorOverlayDesc
	{
		RHI::Device* pDevice = nullptr;
		RHI::DescriptorHeap* pResourceHeap = nullptr;
		RHI::TextureFormat colorFormat = RHI::TextureFormat::RGBA8_UNORM;
		Engine::WindowService* pWindowService = nullptr;
	};

	class EditorOverlay final : public Engine::PresentationOverlay
	{
	public:
		EditorOverlay(const EditorOverlayDesc& desc);
		~EditorOverlay();

		void UpdatePlatform();

		b8 BeginRender(f32 dt);
		b8 EndRender();
		b8 CheckMouseDragging();

		void OnRecord(RHI::CommandList* pCmdList) final;
		void OnFrameEnd() final;

	private:
		void LoadFonts();
		void DefaultStyle();

	private:
		RenderContext m_context;
		UniquePtr<EditorPlatform> m_platform = nullptr;

		RHI::Device* m_device = nullptr;
		RHI::DescriptorHeap* m_resourceHeap = nullptr;

		b8 m_frameReady = false;
	};
}