#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Window/WindowParams.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/PAL/Window/Cursor.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Containers/Vector/ReadOnlyList.h>

namespace Horizon::Engine
{
	class ENGINE_API WindowService : public Service
	{
	public:
		WindowService(const WindowParams& params);
		~WindowService() = default;

		PAL::Window* GetWindow() const noexcept { return m_mainWindow; }
		ReadOnlyList<PAL::Window*> GetWindows() const noexcept { return m_windows; }
		PAL::Cursor* GetCursor() const noexcept { return m_cursor.Get(); }

		PAL::Window* SpawnWindow(const PAL::WindowDesc& desc);
		void DestroyWindow(PAL::Window* pWindow);

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

	private:
		WindowParams m_params;

		UniquePtr<PAL::Cursor> m_cursor;
		PAL::Window* m_mainWindow = nullptr;
		List<PAL::Window*> m_windows;
	};
}