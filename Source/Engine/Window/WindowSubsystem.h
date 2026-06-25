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

		EngineReport OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		Window* m_window;
	};
}