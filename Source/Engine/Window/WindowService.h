#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Window/WindowParams.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/PAL/Window/Cursor.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Containers/ReadOnlyList.h>

namespace Horizon::Engine
{
	class ENGINE_API WindowService : public Service
	{
	public:
		WindowService(const WindowParams& params);
		~WindowService() = default;

		PAL::Window* GetWindow() const noexcept { return m_mainWindow; }
		ReadOnlyList<PAL::Window*> GetWindows() const noexcept { return m_windows; }
		PAL::Cursor* GetCursor() const noexcept { return m_cursor; }

		PAL::Window* SpawnWindow(const PAL::WindowDesc& desc);
		void DestroyWindow(PAL::Window* pWindow);

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

	private:
		WindowParams m_params;

		PAL::Cursor* m_cursor = nullptr;
		PAL::Window* m_mainWindow = nullptr;
		List<PAL::Window*> m_windows;
	};
}