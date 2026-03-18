#include "AssetSystem.h"

#include <Engine/Asset/Builtin/Shader/ShaderAsset.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/VirtualFile/VirtualFileSystem.h>

namespace Horizon
{
    AssetSystem::AssetSystem()
    {
    }

    EngineReport AssetSystem::OnInitialize()
    {
        auto& vfs = RequestSystem<VirtualFileSystem>();
        auto shaderFiles = vfs.ScanFiles("Shaders://");

        auto& jobs = RequestSystem<JobSystem>();
        std::vector<JobHandle> handles;

        for (const auto& physicalPath : shaderFiles)
        {
            if (physicalPath.extension() == ".glsl")
                continue;

            std::string virtualPath = "Shaders://" + physicalPath.filename().string();
            auto asset = std::make_shared<ShaderAsset>(virtualPath, physicalPath);
            m_assets[virtualPath] = asset;

            handles.push_back(jobs.Submit([asset] { asset->Import(); }));
        }

        jobs.Submit([this] { Log::Terminal(LogType::Success, GetObjectType(), "All shaders imported"); }, handles);

        return EngineReport();
    }

    void AssetSystem::OnSync()
    {
    }

    void AssetSystem::OnFinalize()
    {
        for (auto& [key, asset] : m_assets)
            asset->Release();

        m_assets.clear();
    }
}