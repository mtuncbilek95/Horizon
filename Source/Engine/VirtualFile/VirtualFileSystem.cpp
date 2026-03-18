#include "VirtualFileSystem.h"

namespace Horizon
{
    VirtualFileSystem::VirtualFileSystem()
    {
    }

    EngineReport VirtualFileSystem::OnInitialize()
    {
        if (!Mount("Assets", "../assets"))
            return EngineReport(GetObjectType(), "Failed to mount assets");

        if (!Mount("Shaders", "../shaders"))
            return EngineReport(GetObjectType(), "Failed to mount shaders");

        return EngineReport();
    }

    void VirtualFileSystem::OnSync()
    {
    }

    void VirtualFileSystem::OnFinalize()
    {
        if (!Unmount("Assets"))
            Log::Terminal(LogType::Error, GetObjectType(), "Failed to unmount assets");

        if (!Unmount("Shaders"))
            Log::Terminal(LogType::Error, GetObjectType(), "Failed to unmount shaders");

    }

    bool VirtualFileSystem::Mount(const std::string& mountName, const std::filesystem::path& physicalPath)
    {
        if (!std::filesystem::exists(physicalPath))
        {
            std::error_code ec;
            std::filesystem::create_directories(physicalPath, ec);
            if (ec)
            {
                Log::Terminal(LogType::Error, GetObjectType(), "Failed to create directory '{}': {}", physicalPath.string(), ec.message());
                return false;
            }
        }

        m_mountPoints[mountName] = physicalPath;
        Log::Terminal(LogType::Success, GetObjectType(), "Mounted {} successfully!", physicalPath.string());
        return true;
    }

    bool VirtualFileSystem::Unmount(const std::string& mountName)
    {
        auto it = m_mountPoints.find(mountName);
        if (it == m_mountPoints.end())
            return false;

        m_mountPoints.erase(it);
        return true;
    }

    std::filesystem::path VirtualFileSystem::Resolve(const std::string& virtualPath)
    {
        auto separatorPos = virtualPath.find("://");
        if (separatorPos == std::string::npos)
            return virtualPath;

        std::string mountName = virtualPath.substr(0, separatorPos);
        std::string relativePath = virtualPath.substr(separatorPos + 3);

        auto it = m_mountPoints.find(mountName);
        if (it == m_mountPoints.end())
            return {};

        return it->second / relativePath;
    }

    std::vector<std::filesystem::path> VirtualFileSystem::ScanFiles(const std::string& virtualPath)
    {
        std::filesystem::path absPath = Resolve(virtualPath);
        std::vector<std::filesystem::path> result;

        if (!std::filesystem::exists(absPath))
            return result;

        for (auto& entry : std::filesystem::recursive_directory_iterator(absPath))
            if (entry.is_regular_file())
                result.push_back(entry.path());

        return result;
    }

    std::vector<std::filesystem::path> VirtualFileSystem::ScanFiles(const std::string& virtualPath, const std::string& extension)
    {
        std::filesystem::path absPath = Resolve(virtualPath);
        std::vector<std::filesystem::path> result;

        if (!std::filesystem::exists(absPath))
            return result;

        for (auto& entry : std::filesystem::recursive_directory_iterator(absPath))
            if (entry.is_regular_file() && entry.path().extension() == extension)
                result.push_back(entry.path());

        return result;
    }
}