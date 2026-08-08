#pragma once

#include <Engine/Core/AppModule.h>

namespace Horizon::Engine
{
	class Application;

	class H_EXPORT System : public AppModule
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