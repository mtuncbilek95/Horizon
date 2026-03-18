#pragma once

#include <Engine/System/System.h>
#include <Engine/EntityComponent/EntitySystem.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace Horizon
{
    class EntityComponentSystem final : public System<EntityComponentSystem>
    {
    public:
        EntityComponentSystem();
        ~EntityComponentSystem() = default;

        template<typename T, typename... Args, typename = std::enable_if_t<std::derived_from<T, EntitySystem>>>
        T& AddSystem(Args&&... args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            system->SetEngine(GetEngine());
            auto& ref = *system;
            m_entitySystems[std::type_index(typeid(T))] = std::move(system);
            return ref;
        }

        template<typename T, typename = std::enable_if_t<std::derived_from<T, EntitySystem>>>
        void RemoveSystem()
        {
            m_entitySystems.erase(std::type_index(typeid(T)));
        }

        template<typename T, typename = std::enable_if_t<std::derived_from<T, EntitySystem>>>
        T& GetSystem()
        {
            auto typeIndex = std::type_index(typeid(T));
            if (!m_entitySystems.contains(typeIndex))
                throw std::runtime_error("EntitySystem not found");

            return *static_cast<T*>(m_entitySystems[typeIndex].get());
        }

    private:
        EngineReport OnInitialize();
        void OnSync();
        void OnFinalize();

    private:
        std::unordered_map<std::type_index, std::unique_ptr<EntitySystem>> m_entitySystems;
    };
}
