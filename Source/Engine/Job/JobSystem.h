#pragma once

#include <Engine/Core/System.h>

namespace Horizon
{
	class JobSystem : public System<JobSystem>
	{
	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;
	};
}