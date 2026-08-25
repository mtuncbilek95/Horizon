#pragma once

#include <Editor/Models/SelectionModel.h>
#include <Engine/Core/Service.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon::RHI
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

		ViewRegistry* GetViewRegistry() const { return m_viewRegistry; }

	private:
		PAL::Window* m_engineWindow = nullptr;
		EditorRenderer* m_editorRenderer = nullptr;

		ViewRegistry* m_viewRegistry = nullptr;
		MenuRegistry* m_menuRegistry = nullptr;

		// The ones below are most probably temporary
		RHI::GfxFence* m_fence = nullptr;
		RHI::GfxQueue* m_queue = nullptr;
		RHI::GfxSwapchain* m_swapchain = nullptr;

		SelectionModel m_selection;
	};
}