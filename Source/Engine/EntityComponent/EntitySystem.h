#pragma once

#include <Engine/Engine/Engine.h>

namespace Horizon
{
    class EntitySystem
    {
    public:
        EntitySystem() = default;
        virtual ~EntitySystem() = default;

        Engine* GetEngine() const { return m_mainEngine; }
        void SetEngine(Engine* pEngine) { m_mainEngine = pEngine; }

        template<typename U>
        U& RequestSystem()
        {
            if (!m_mainEngine)
                throw std::runtime_error("System is not attached to an engine");

            return m_mainEngine->GetSystem<U>();
        }

        virtual void OnStart() = 0;
        virtual void OnTick() = 0;
        virtual void OnStop() = 0;

    private:
        Engine* m_mainEngine = nullptr;
    };
}