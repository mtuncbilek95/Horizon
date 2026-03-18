#pragma once

#include <Engine/System/System.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Horizon
{
    class VirtualFileSystem final : public System<VirtualFileSystem>
    {
    public:
        VirtualFileSystem();
        ~VirtualFileSystem() = default;

        bool Mount(const std::string& mountName, const std::filesystem::path& physicalPath);
        bool Unmount(const std::string& mountName);
        std::filesystem::path Resolve(const std::string& virtualPath);

        std::vector<std::filesystem::path> ScanFiles(const std::string& virtualPath);
        std::vector<std::filesystem::path> ScanFiles(const std::string& virtualPath, const std::string& extension);

    private:
        EngineReport OnInitialize() final;
        void OnSync() final;
        void OnFinalize() final;

    private:
        std::unordered_map<std::string, std::filesystem::path> m_mountPoints;
    };
}