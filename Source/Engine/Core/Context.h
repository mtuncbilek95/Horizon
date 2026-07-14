#pragma once

#include <Engine/Core/EngineModule.h>

namespace Horizon
{
	class Engine;

	class H_EXPORT Context : public EngineModule
	{
	public:
		Context() = default;
		virtual ~Context() = default;

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};
}