#include <Engine/Core/Engine.h>
#include <Engine/Cache/CacheContext.h>
#include <Engine/Dummy/DummyContext.h>
#include <Engine/Graphics/GfxContext.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/World/EntityComponentSystem.h>
#include <Engine/Presentation/PresentationSystem.h>

using namespace Horizon;

int main(int argc, char* argv[])
{
	Engine engine;

	engine.AddContext<CacheContext>();
	engine.AddContext<GfxContext>();
	engine.AddContext<DummyContext>();

	engine.AddSystem<WindowSystem>();
	engine.AddSystem<EntityComponentSystem>();
	engine.AddSystem<PresentationSystem>();

	engine.Run();
}