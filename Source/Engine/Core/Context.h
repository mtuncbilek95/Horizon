#pragma once

#include <Engine/Core/AppModule.h>

namespace Horizon::Engine
{
	class Application;

	class H_EXPORT Context : public AppModule
	{
	public:
		Context() = default;
		virtual ~Context() = default;

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};
}