#pragma once

#include <Engine/Core/Submodule.h>

namespace Horizon
{
	class Window;

	class WindowModule final : public Submodule
	{
	public:
		WindowModule() = default;
		~WindowModule() = default;

		Window* GetWindow() const { return m_window; }

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
		Window* m_window;
	};
}