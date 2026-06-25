#pragma once

#include <Engine/Core/Subsystem.h>

namespace Horizon
{
	class GfxSwapchain;

	class PresentationSubsystem final : public Subsystem
	{
	public:
		PresentationSubsystem() = default;
		~PresentationSubsystem() = default;

		EngineReport OnAttach(Engine* engine) final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		GfxSwapchain* m_swapchain = nullptr;
	};
}