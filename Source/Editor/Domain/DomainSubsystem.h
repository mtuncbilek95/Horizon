#pragma once

#include <Engine/Core/Subsystem.h>

namespace Horizon
{
	class H_EXPORT DomainSubsystem : public Subsystem
	{
	public:
		DomainSubsystem() = default;
		~DomainSubsystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
	};
}