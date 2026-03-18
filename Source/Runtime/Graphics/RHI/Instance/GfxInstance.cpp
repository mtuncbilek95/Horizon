#include "GfxInstance.h"

#include <Runtime/Graphics/Vulkan/Instance/GfxVkInstance.h>

namespace Horizon
{
    std::shared_ptr<GfxInstance> GfxInstance::Create(const GfxInstanceDesc& desc)
    {
        switch(desc.type)
        {
            case GfxType::Vulkan: 
                return std::make_shared<GfxVkInstance>(desc);
            case GfxType::DX12:   
                return nullptr; // TODO: Implement DX12 instance.
            default:             
                return nullptr;
        }
    }

    GfxInstance::GfxInstance(const GfxInstanceDesc& desc) : m_desc(desc)
    {
    }
}
