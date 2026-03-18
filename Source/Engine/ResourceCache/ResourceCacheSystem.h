#pragma once

#include <Engine/System/System.h>

namespace Horizon
{
    class ResourceCacheSystem : public System<ResourceCacheSystem>
    {
    public:
        ResourceCacheSystem();
        ~ResourceCacheSystem() = default;

    private:
        EngineReport OnInitialize();
        void OnSync();
        void OnFinalize();

    private:
    };
}