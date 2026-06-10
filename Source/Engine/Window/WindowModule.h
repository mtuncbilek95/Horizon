#pragma once

#include <Runtime/Window/Window.h>
#include <Engine/Engine/IModule.h>

namespace Horizon
{
	class WindowModule : public IModule
	{
	public:
		WindowModule() = default;
		WindowModule(const WindowDesc& desc);
		~WindowModule();

		const Window& GetMainWindow() const { return *m_window; }

		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
		WindowDesc m_desc = WindowDesc();
		std::unique_ptr<Window> m_window;
	};
}