#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/EditorOverlay.h>
#include <Editor/MainMenu/MenuRegistry.h>
#include <Editor/Toolbar/ToolBarRegistry.h>
#include <Editor/Views/ViewRegistry.h>
#include <Engine/Core/Service.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/PAL/Window/Cursor.h>

namespace Horizon::RHI
{
	class Device;
}

namespace Horizon::Engine
{
	class PresentationService;
}

namespace Horizon::Editor
{
	class EditorOverlay;
	class EditorContext;

	class EditorService final : public Engine::Service
	{
	public:
		Engine::ModuleReport OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::ModuleDependency& graph) final;

		ViewRegistry* GetViewRegistry() const { return m_viewRegistry.Get(); }

	private:
		PAL::Window* m_engineWindow = nullptr;
		PAL::Cursor* m_engineCursor = nullptr;
		UniquePtr<EditorOverlay> m_editorOverlay = nullptr;

		UniquePtr<MenuRegistry> m_menuRegistry = nullptr;
		UniquePtr<ToolBarRegistry> m_toolRegistry = nullptr;
		UniquePtr<ViewRegistry> m_viewRegistry = nullptr;

		RHI::Device* m_device = nullptr;
		Engine::PresentationService* m_presentation = nullptr;

		EditorContext m_editorContext;
	};
}