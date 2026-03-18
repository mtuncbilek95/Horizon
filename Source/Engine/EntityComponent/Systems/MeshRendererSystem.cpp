#include "MeshRendererSystem.h"

namespace Horizon
{
    MeshRendererSystem::MeshRendererSystem()
    {
    }

    void MeshRendererSystem::OnStart()
    {
        Log::Terminal(LogType::Debug, "MeshRendererSystem", "OnStart");
    }

    void MeshRendererSystem::OnTick()
    {
    }

    void MeshRendererSystem::OnStop()
    {
    }
}