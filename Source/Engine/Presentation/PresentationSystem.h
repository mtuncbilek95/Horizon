#pragma once

#include <Engine/System/System.h>

namespace Horizon
{
    class GfxSwapchain;

    class PresentationSystem final : public System<PresentationSystem>
    {
    public:
        PresentationSystem();
        ~PresentationSystem() = default;

        GfxSwapchain& Swapchain() const { return *m_swapchain.get(); }

    private:
        EngineReport OnInitialize() final;
        void OnSync() final;
        void OnFinalize() final;

    private:
        std::shared_ptr<GfxSwapchain> m_swapchain;
    };
}