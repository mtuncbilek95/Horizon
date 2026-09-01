#include "WindowService.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	ModuleReport WindowService::OnInitialize()
	{
		// TODO: Those values will come from a different place later.

		// Create Window.
		PAL::WindowDesc winDesc = {};
		winDesc.width = 1920;
		winDesc.height = 1080;
		winDesc.mode = PAL::WindowMode::Windowed;
		winDesc.titleName = "Horizon Engine";


		m_window = Memory::Allocator::Create<PAL::Window>(Memory::CurrLoc(), winDesc);
		if (!m_window)
			return ModuleReport("Window has not been initialize.");

		m_window->Show();
		Terminal::Info(StringOps::GetName(this), "Window has been initialized!");

		return ModuleReport();
	}

	void WindowService::OnExecute()
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
		// TODO: width and height should come from a file?
	}
}