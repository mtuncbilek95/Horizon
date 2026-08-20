#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon
{
	class GfxFence;
	class GfxQueue;
	class GfxSwapchain;
}

namespace Horizon::Editor
{
	class EditorRenderer;

	class ViewRegistry;
	class MenuRegistry;

	class H_EXPORT EditorService final : public Engine::Service
	{
	public:
		EditorService() = default;
		~EditorService() = default;

		Engine::ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;

		void DeclareDependencies(Engine::ModuleGraph& graph) final;

	private:
		PAL::Window* m_engineWindow = nullptr;
		EditorRenderer* m_editorRenderer = nullptr;

		ViewRegistry* m_viewRegistry = nullptr;
		MenuRegistry* m_menuRegistry = nullptr;

		// The ones below are most probably temporary
		GfxFence* m_fence = nullptr;
		GfxQueue* m_queue = nullptr;
		GfxSwapchain* m_swapchain = nullptr;
	};
}