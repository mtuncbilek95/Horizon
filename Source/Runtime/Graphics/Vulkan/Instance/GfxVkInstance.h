#pragma once

#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>

#include <vulkan/vulkan.h>

#include <memory>

namespace Horizon
{
    class GfxVkInstance final : public GfxInstance
    {
    public:
        GfxVkInstance(const GfxInstanceDesc& desc);
        ~GfxVkInstance() override final;

        std::shared_ptr<GfxDevice> CreateDevice(const GfxDeviceDesc& desc) override final;
        void* Instance() const override final;
        void* Adapter() const override final;
        
    private:
        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;

#if defined(HORIZON_DEBUG)
        VkDebugUtilsMessengerEXT m_debugMessenger;
#endif
    };
}
