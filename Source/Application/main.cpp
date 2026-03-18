#include <Engine/Engine/Engine.h>

#include <Engine/Job/JobSystem.h>
#include <Engine/Window/WindowSystem.h>
#include <Engine/Input/InputSystem.h>
#include <Engine/VirtualFile/VirtualFileSystem.h>
#include <Engine/Asset/AssetSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/EntityComponent/EntityComponentSystem.h>
#include <Engine/Game/GameSystem.h>

using namespace Horizon;

int main(int argc, char* argv[])
{
    Engine newEngine;

    newEngine.AddSystem<JobSystem>();
    newEngine.AddSystem<WindowSystem>();
    newEngine.AddSystem<InputSystem>();
    newEngine.AddSystem<VirtualFileSystem>();
    newEngine.AddSystem<AssetSystem>();
    newEngine.AddSystem<GraphicsSystem>();
    newEngine.AddSystem<PresentationSystem>();
    newEngine.AddSystem<EntityComponentSystem>();
    newEngine.AddSystem<GameSystem>();

    newEngine.Run();

    return 0;
}