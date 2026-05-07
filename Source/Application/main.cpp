#include <Application/Engine/Engine.h>
#include <Application/Systems/Window/WindowSystem.h>
#include <Application/Systems/Renderer/RenderSystem.h>
#include <Application/Systems/ECS/EntityComponentSystem.h>
#include <Application/Systems/Dummy/DummySystem.h>

using namespace Horizon;

int main(int argc, char* argv[])
{
	Engine newEngine;

	newEngine.AddSystem<WindowSystem>();
	newEngine.AddSystem<RenderSystem>();
	newEngine.AddSystem<EntityComponentSystem>();
	newEngine.AddSystem<DummySystem>();

	newEngine.Run();
}