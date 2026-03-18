#pragma once

#include <filesystem>
#include <string>

namespace Horizon
{
    class Asset
    {
    public:
        Asset(const std::string& virtualPath, const std::filesystem::path& physicalPath)
            : m_virtualPath(virtualPath), m_physicalPath(physicalPath), m_imported(false) {}

        virtual ~Asset() = default;

        virtual void Import() = 0;
        virtual void Release() = 0;

        bool IsImported() const { return m_imported; }
        const std::string& VirtualPath() const { return m_virtualPath; }
        const std::filesystem::path& PhysicalPath() const { return m_physicalPath; }

    protected:
        std::string m_virtualPath;
        std::filesystem::path m_physicalPath;
        bool m_imported;
    };
}