#pragma once

#include <Editor/Renderer/EditorContext.h>
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
	class EditorContext;
	class ViewRegistry;
	class MenuRegistry;
	class ToolBarRegistry;

	class EDITOR_API EditorService final : public Engine::Service
	{
	public:
		EditorService() = default;
		~EditorService() = default;

		Engine::ModuleReport OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

		ViewRegistry* GetViewRegistry() const { return m_viewRegistry; }

	private:
		PAL::Window* m_engineWindow = nullptr;
		EditorRenderer* m_editorRenderer = nullptr;

		MenuRegistry* m_menuRegistry = nullptr;
		ToolBarRegistry* m_toolRegistry = nullptr;
		ViewRegistry* m_viewRegistry = nullptr;

		RHI::GfxFence* m_fence = nullptr;
		RHI::GfxQueue* m_queue = nullptr;
		RHI::GfxSwapchain* m_swapchain = nullptr;

		EditorContext m_editorContext;
	};
}