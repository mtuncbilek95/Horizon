#pragma once

#include <Engine/EntityComponent/EntitySystem.h>

namespace Horizon
{
    class CameraSystem : public EntitySystem
    {
    public:
        CameraSystem();
        ~CameraSystem() = default;

        void OnStart() final;
        void OnTick() final;
        void OnStop() final;
    };
}