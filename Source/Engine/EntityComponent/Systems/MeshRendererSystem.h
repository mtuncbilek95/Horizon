#pragma once

#include <Engine/EntityComponent/EntitySystem.h>

namespace Horizon
{
    class MeshRendererSystem final : public EntitySystem
    {
    public:
        MeshRendererSystem();
        ~MeshRendererSystem() = default;

        void OnStart() final;
        void OnTick() final;
        void OnStop() final;

    private:
    };
}