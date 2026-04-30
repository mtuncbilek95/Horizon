#pragma once

#include <Runtime/MeshLoader/MeshData.h>

#include <filesystem>

namespace Horizon
{
    struct MeshLoader
    {
        static SceneData Load(const std::filesystem::path& path);
    };
}
