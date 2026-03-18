#pragma once

#include <Engine/Asset/Asset.h>
#include <Engine/System/System.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace Horizon
{
    class AssetSystem final : public System<AssetSystem>
    {
    public:
        AssetSystem();
        ~AssetSystem() = default;

        template<typename T>
        T* GetAsset(const std::string& virtualPath)
        {
            auto it = m_assets.find(virtualPath);
            if (it == m_assets.end())
                return nullptr;

            return static_cast<T*>(it->second.get());
        }

    private:
        EngineReport OnInitialize() final;
        void OnSync() final;
        void OnFinalize() final;

    private:
        std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;
    };
}