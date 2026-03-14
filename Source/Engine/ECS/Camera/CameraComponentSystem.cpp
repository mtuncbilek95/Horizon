#include "CameraComponentSystem.h"

namespace Horizon
{
	SystemReport CameraComponentSystem::OnStart()
	{
		// Log::Terminal(LogType::Success, GetObjectType(), "CameraComponentSystem started.");
		return SystemReport();
	}

	void CameraComponentSystem::OnTick()
	{
		// Log::Terminal(LogType::Debug, GetObjectType(), "CameraComponentSystem tick.");
	}

	void CameraComponentSystem::OnStop()
	{
		// Log::Terminal(LogType::Warning, GetObjectType(), "CameraComponentSystem stopped.");
	}
}