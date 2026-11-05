#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Engine/Core/Service.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/PAL/Window/Cursor.h>

namespace Horizon::RHI
{
	class Device;
	class Swapchain;
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
		void DeclareDependencies(Engine::ModuleDependency& graph) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

		ViewRegistry* GetViewRegistry() const { return m_viewRegistry.Get(); }

	private:
		PAL::Window* m_engineWindow = nullptr;
		PAL::Cursor* m_engineCursor = nullptr;
		UniquePtr<EditorRenderer> m_editorRenderer = nullptr;

		UniquePtr<MenuRegistry> m_menuRegistry = nullptr;
		UniquePtr<ToolBarRegistry> m_toolRegistry = nullptr;
		UniquePtr<ViewRegistry> m_viewRegistry = nullptr;

		RHI::Device* m_device = nullptr;
		RHI::Swapchain* m_swapchain = nullptr;

		EditorContext m_editorContext;
	};
}