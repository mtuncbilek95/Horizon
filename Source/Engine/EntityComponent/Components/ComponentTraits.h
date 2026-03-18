#pragma once

#include <Engine/EntityComponent/Components/TransformComponent.h>
#include <Engine/EntityComponent/Components/MeshComponent.h>
#include <Engine/EntityComponent/Components/CameraComponent.h>

namespace Horizon
{
    template<typename T>
    struct ComponentTraits;

    template<> 
    struct ComponentTraits<TransformComponent> 
    { 
        static constexpr const char* JsonKey = "transform"; 
    };

    template<> 
    struct ComponentTraits<MeshComponent> 
    { 
        static constexpr const char* JsonKey = "mesh"; 
    };

    template<> 
    struct ComponentTraits<CameraComponent> 
    { 
        static constexpr const char* JsonKey = "camera"; 
    };
}