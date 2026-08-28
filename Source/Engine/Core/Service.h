#pragma once

#include <Engine/Core/Module.h>

namespace Horizon::Engine
{
	class H_EXPORT Service : public Module
	{
	public:
		Service() = default;
		virtual ~Service() = default;
		virtual void OnExecute() = 0;
	};
}