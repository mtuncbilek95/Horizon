#pragma once

#include <Runtime/Window/BasicWindow.h>
#include <Engine/System/System.h>

namespace Horizon
{
	class WindowSystem : public System<WindowSystem>
	{
		using WindowHandle = void*;
	public:
		WindowSystem() = default;
		virtual ~WindowSystem() = default;

		const Math::Vec2u& GetWindowSize() const { return m_window->GetSize(); }
		WindowHandle GetWindowHandle() const { return m_window->GetHandle(); }
		WindowHandle GetAPIWindow() const { return m_window->GetNativeWindow(); }

		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;

	private:
		std::unique_ptr<BasicWindow> m_window;
	};
}