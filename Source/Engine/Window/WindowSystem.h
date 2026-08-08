#pragma once

#include <Engine/Core/System.h>

#include <Runtime/PAL/Window/Window.h>

namespace Horizon::Engine
{
	class H_EXPORT WindowSystem final : public System
	{
	public:
		WindowSystem() = default;
		~WindowSystem() = default;

		PAL::Window* GetWindow() const { return m_window; }

		AppReport OnAttach(Application* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		PAL::Window* m_window = nullptr;
	};
}