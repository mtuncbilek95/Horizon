#pragma once

#include <Engine/Core/Service.h>
#include <Engine/Window/WindowParams.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon::Engine
{
	class ENGINE_API WindowService : public Service
	{
	public:
		WindowService(const WindowParams& params);
		~WindowService() = default;

		PAL::Window* GetWindow() const noexcept { return m_window; }

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final {}
		void OnLibraryUnregistered(const ReflectionLibrary& library) final {}

	private:
		WindowParams m_params;
		PAL::Window* m_window = nullptr;
	};
}