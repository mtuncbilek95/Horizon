#pragma once

#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <filesystem>

namespace Horizon
{
    struct ShaderData
    {
        std::string sourceText;
        std::string entryPoint;
        ShaderStage stage;
    };

    struct ShaderCompiler final
    {
        static ReadArray<u32> GenerateSpirv(const std::string& source, const std::string& entryPoint, ShaderStage stage);
        static ShaderData ReadShaderData(const std::filesystem::path& path);
    };
}
