#pragma once

#include <Engine/Core/Subsystem.h>

namespace Horizon
{
	class Window;

	class WindowSubsystem final : public Subsystem
	{
	public:
		WindowSubsystem() = default;
		~WindowSubsystem() = default;

		Window* GetWindow() const { return m_window; }

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetExecuteAfter(std::vector<std::type_index>& out) const final;
		void GetExecuteBefore(std::vector<std::type_index>& out) const final;

	private:
		Window* m_window;
	};
}