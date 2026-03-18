#include <Engine/Engine/Engine.h>

#include <Engine/Job/JobSystem.h>
#include <Engine/Window/WindowSystem.h>
#include <Engine/VirtualFile/VirtualFileSystem.h>
#include <Engine/Asset/AssetSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/Presentation/PresentationSystem.h>

using namespace Horizon;

int main(int argc, char* argv[])
{
    Engine newEngine;

    newEngine.AddSystem<JobSystem>();
    newEngine.AddSystem<WindowSystem>();
    newEngine.AddSystem<VirtualFileSystem>();
    newEngine.AddSystem<AssetSystem>();
    newEngine.AddSystem<GraphicsSystem>();
    newEngine.AddSystem<PresentationSystem>();

    newEngine.Run();

    return 0;
}