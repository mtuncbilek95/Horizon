#pragma once

namespace Horizon
{
    typedef void* GfxHandle;

    class GfxDevice;

    class GfxObject
    {
    public:
        GfxObject(GfxDevice* pDevice) : m_owner(pDevice) {}
        virtual ~GfxObject() = default;

        GfxDevice* Root() const { return m_owner; }

    private:
        GfxDevice* m_owner;
    };
}
