#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon::Engine
{
	class H_EXPORT WindowService : public Service
	{
	public:
		WindowService() = default;
		~WindowService() = default;

		PAL::Window* GetWindow() const noexcept { return m_window; }

		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

	private:
		PAL::Window* m_window = nullptr;
	};
}