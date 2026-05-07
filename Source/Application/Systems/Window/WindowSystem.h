#pragma once

#include <Runtime/Window/Window.h>
#include <Application/Engine/System.h>

namespace Horizon
{
	class WindowSystem final : public System<WindowSystem>
	{
		using WindowHandle = void*;
	public:
		WindowSystem() = default;
		virtual ~WindowSystem() = default;

		Window& MainWindow() const { return *m_window.get(); }
		const Math::Vec2u& WindowSize() const { return m_window->GetSize(); }
		WindowHandle Handle() const { return m_window->GetHandle(); }
		WindowHandle Instance() const { return m_window->GetInstance(); }
		WindowHandle APIWindow() const { return m_window->GetNativeWindow(); }

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		std::unique_ptr<Window> m_window;
	};
}