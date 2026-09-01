#pragma once

#include <Engine/Core/Module.h>
#include <Engine/Core/EngineFrame.h>

namespace Horizon::Engine
{
	class H_EXPORT Service : public Module
	{
	public:
		Service() = default;
		virtual ~Service() = default;
		virtual void OnExecute(const EngineFrame& ctx) = 0;
	};
}