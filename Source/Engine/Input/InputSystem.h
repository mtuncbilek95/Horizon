#pragma once

#include <Engine/System/System.h>
#include <Runtime/Input/InputEvents.h>

#include <functional>

namespace Horizon
{
    class InputDispatcher;

    class InputSystem final : public System<InputSystem>
    {
    public:
        InputSystem();
        ~InputSystem() = default;

        void RegisterInput(InputType type, std::function<void(const InputMessage&)> callback);

    private:
        EngineReport OnInitialize() final;
        void OnSync() final;
        void OnFinalize() final;

    private:
        InputDispatcher* m_dispatcher;
    };
}