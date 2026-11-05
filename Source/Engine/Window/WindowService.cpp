#include "WindowService.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	WindowService::WindowService(const WindowParams& params) : m_params(params)
	{
	}

	ModuleReport WindowService::OnInitialize()
	{
		m_cursor = MakeUnique<PAL::Cursor>();
		if (!m_cursor)
			return ModuleReport("Cursor has not been initialize.");

		// TODO: Add Config later

		// Create Window.
		PAL::WindowDesc winDesc = {};
		winDesc.width = 1920;
		winDesc.height = 1080;
		winDesc.mode = PAL::WindowMode::Windowed;
		winDesc.titleName = "Horizon Engine";
		winDesc.flags = m_params.flags;

		m_mainWindow = SpawnWindow(winDesc);
		if (!m_mainWindow)
			return ModuleReport("Window has not been initialize.");

		m_mainWindow->Show();
		Terminal::Info(StringHelper::GetTypeName(this), "Window has been initialized!");

		return ModuleReport();
	}

	void WindowService::OnExecute(const EngineFrame& ctx)
	{
		if (!m_mainWindow)
			return;

		for (PAL::Window* pWindow : m_windows)
			pWindow->ClearMessages();

		PAL::Window::PumpMessages();
		m_cursor->Update();

		if (!m_mainWindow->GetActive())
			GetEngine()->RequestExit("Main window is no longer active!");
	}

	void WindowService::OnFinalize()
	{
		while (!m_windows.IsEmpty())
			DestroyWindow(m_windows.Back());

		m_mainWindow = nullptr;

		m_cursor = nullptr;
	}

	PAL::Window* WindowService::SpawnWindow(const PAL::WindowDesc& desc)
	{
		PAL::Window* pWindow = new PAL::Window(desc);

		if (!pWindow)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to spawn window '{}'.", desc.titleName);
			return nullptr;
		}

		pWindow->AddListener(m_cursor.Get());
		m_windows.PushBack(pWindow);

		return pWindow;
	}

	void WindowService::DestroyWindow(PAL::Window* pWindow)
	{
		if (!pWindow)
			return;

		if (!m_windows.Remove(pWindow))
		{
			Terminal::Warn(StringHelper::GetTypeName(this), "Window '{}' is not owned by WindowService.", pWindow->GetName());
			return;
		}

		if (pWindow == m_mainWindow)
			m_mainWindow = nullptr;

		delete pWindow;
	}

	void WindowService::DeclareDependencies(ModuleDependency& graph)
	{
	}

	void WindowService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void WindowService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}
}