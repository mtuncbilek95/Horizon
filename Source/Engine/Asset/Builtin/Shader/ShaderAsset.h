#pragma once

#include <Runtime/Data/Containers/ReadArray.h>
#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <Engine/Asset/Asset.h>

namespace Horizon
{
    class ShaderAsset final : public Asset
    {
    public:
        ShaderAsset(const std::string& virtualPath, const std::filesystem::path& physicalPath);
        ~ShaderAsset() override = default;

        void Import() final;
        void Release() final;

        ShaderStage Stage() const { return m_stage; }
        const std::string& EntryPoint() const { return m_entryPoint; }
        const ReadArray<u32>& SpirV() const { return m_spirv; }

    private:
        ShaderStage m_stage;
        std::string m_entryPoint;
        ReadArray<u32> m_spirv;
    };
}
