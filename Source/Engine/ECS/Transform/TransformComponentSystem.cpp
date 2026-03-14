#include "TransformComponentSystem.h"

namespace Horizon
{
	SystemReport TransformComponentSystem::OnStart()
	{
		// Log::Terminal(LogType::Success, GetObjectType(), "TransformComponentSystem started.");
		return SystemReport();
	}

	void TransformComponentSystem::OnTick()
	{
		// Log::Terminal(LogType::Debug, GetObjectType(), "TransformComponentSystem tick.");
	}

	void TransformComponentSystem::OnStop()
	{
		// Log::Terminal(LogType::Warning, GetObjectType(), "TransformComponentSystem stopped.");
	}
}