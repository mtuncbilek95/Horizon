#include "InputSystem.h"

#include <Runtime/Window/BasicWindow.h>
#include <Engine/Window/WindowSystem.h>

namespace Horizon
{
    InputSystem::InputSystem()
    {
    }

    void InputSystem::RegisterInput(InputType type, std::function<void(const InputMessage&)> callback)
    {
        switch (type)
        {
        case InputType::Key:
            m_dispatcher->OnKeyPressed(std::move(callback));
            break;
        case InputType::MouseButton:
            m_dispatcher->OnMousePressed(std::move(callback));
            break;
        default:
            break;
        }
    }

    EngineReport InputSystem::OnInitialize()
    {
        auto& wSystem = RequestSystem<WindowSystem>();
        auto& pWindow = wSystem.Window();

        m_dispatcher = &pWindow.GetInputDispatcher();

        if(!m_dispatcher)
            return EngineReport(GetObjectType(), "Failed to get input dispatcher");

        return EngineReport();
    }

    void InputSystem::OnSync()
    {
    }

    void InputSystem::OnFinalize()
    {
    }
}