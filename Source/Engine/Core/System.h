#pragma once

#include <Engine/Core/EngineModule.h>

namespace Horizon
{
	class Engine;

	class H_EXPORT System : public EngineModule
	{
	public:
		System() = default;
		virtual ~System() = default;

		System(const System&) = delete;
		System& operator=(const System&) = delete;

		virtual void OnSync() {}

		virtual void GetExecutionOrder(OrderRules& rules) const {}
	};
}