#include "CameraSystem.h"

namespace Horizon
{
    CameraSystem::CameraSystem()
    {
    }

    void CameraSystem::OnStart()
    {
         Log::Terminal(LogType::Debug, "CameraSystem", "OnStart");
    }

    void CameraSystem::OnTick()
    {
    }

    void CameraSystem::OnStop()
    {
    }
}