#include "WindowService.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	WindowService::WindowService(const WindowParams& params) : m_params(params)
	{
	}

	ModuleReport WindowService::OnInitialize()
	{
		// TODO: Add Config later

		// Create Window.
		PAL::WindowDesc winDesc = {};
		winDesc.width = 1920;
		winDesc.height = 1080;
		winDesc.mode = PAL::WindowMode::Windowed;
		winDesc.titleName = "Horizon Engine";
		winDesc.flags = m_params.flags;

		m_window = Memory::Allocator::Create<PAL::Window>(Memory::CurrLoc(), winDesc);
		if (!m_window)
			return ModuleReport("Window has not been initialize.");

		m_window->Show();
		Terminal::Info(StringOps::GetName(this), "Window has been initialized!");

		return ModuleReport();
	}

	void WindowService::OnExecute(const EngineFrame& ctx)
	{
		if (!m_window)
			return;

		m_window->PollEvents();

		if (!m_window->GetActive())
			GetEngine()->RequestExit("Main window is no longer active!");
	}

	void WindowService::OnFinalize()
	{
		if (!m_window)
			return;

		Memory::Allocator::Delete(m_window);
		m_window = nullptr;
	}

	void WindowService::DeclareDependencies(ModuleGraph& graph)
	{
	}
}