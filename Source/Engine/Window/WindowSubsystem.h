#pragma once

#include <Engine/Core/Subsystem.h>

#include <Runtime/PAL/Window/Window.h>

namespace Horizon
{
	class H_EXPORT WindowSubsystem final : public Subsystem
	{
	public:
		WindowSubsystem() = default;
		~WindowSubsystem() = default;

		PAL::Window* GetWindow() const { return m_window; }

		EngineReport OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		PAL::Window* m_window;
	};
}