#include "ShaderCompiler.h"

#include <Runtime/Data/DataReader.h>

#include <magic_enum/magic_enum.hpp>
#include <shaderc/shaderc.hpp>

#include <unordered_map>
#include <vector>

namespace Horizon
{
    static shaderc_shader_kind ToShadercKind(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:   return shaderc_vertex_shader;
        case ShaderStage::Fragment: return shaderc_fragment_shader;
        case ShaderStage::Compute:  return shaderc_compute_shader;
        case ShaderStage::Geometry: return shaderc_geometry_shader;
        case ShaderStage::TessControl: return shaderc_tess_control_shader;
        case ShaderStage::TessEval: return shaderc_tess_evaluation_shader;
        case ShaderStage::Raygen: return shaderc_raygen_shader;
        case ShaderStage::Anyhit: return shaderc_anyhit_shader;
        case ShaderStage::ClosestHit: return shaderc_closesthit_shader;
        case ShaderStage::Miss: return shaderc_miss_shader;
        case ShaderStage::Intersection: return shaderc_intersection_shader;
        case ShaderStage::Callable: return shaderc_callable_shader;
        case ShaderStage::Task: return shaderc_task_shader;
        case ShaderStage::Mesh: return shaderc_mesh_shader;
        default:
            Log::Terminal(LogType::Fatal, "ShaderCompiler", "Unsupported ShaderStage: {}", magic_enum::enum_name(stage));
            return shaderc_vertex_shader;
        }
    }

    ReadArray<u32> ShaderCompiler::GenerateSpirv(const std::string& source, const std::string& entryPoint, ShaderStage stage)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            source,
            ToShadercKind(stage),
            entryPoint.c_str(),
            options
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            Log::Terminal(LogType::Fatal, "ShaderCompiler", "SPIR-V compilation failed: {}", result.GetErrorMessage());
        }

        std::vector<u32> spirv(result.cbegin(), result.cend());
        return ReadArray<u32>(spirv.data(), static_cast<u32>(spirv.size()));
    }
    
    ShaderData ShaderCompiler::ReadShaderData(const std::filesystem::path& path)
    {
        static const std::unordered_map<std::string, ShaderStage> s_extToStage =
        {
            { ".vert", ShaderStage::Vertex },
            { ".frag", ShaderStage::Fragment },
            { ".comp", ShaderStage::Compute },
            { ".geom", ShaderStage::Geometry },
            { ".tesc", ShaderStage::TessControl },
            { ".tese", ShaderStage::TessEval },
            { ".mesh", ShaderStage::Mesh },
            { ".task", ShaderStage::Task },
            { ".rgen", ShaderStage::Raygen },
            { ".rahit", ShaderStage::Anyhit },
            { ".rchit", ShaderStage::ClosestHit },
            { ".rmiss", ShaderStage::Miss },
            { ".rint", ShaderStage::Intersection },
            { ".rcall", ShaderStage::Callable },
        };

        std::string ext = path.extension().string();
        auto it = s_extToStage.find(ext);
        if (it == s_extToStage.end())
        {
            Log::Terminal(LogType::Fatal, "ShaderCompiler", "Unknown shader extension: {}", ext);
            return ShaderData();
        }

        ShaderData data;
        data.sourceText = DataReader::ReadToTextFile(path);
        data.entryPoint = "main";
        data.stage = it->second;
        return data;
    }
}
