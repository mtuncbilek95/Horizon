#include <Engine/Core/Engine.h>

#include <Engine/Cache/CacheContext.h>
#include <Engine/Graphics/GfxContext.h>
#include <Engine/Graphics/SamplerContext.h>
#include <Engine/Graphics/ShaderContext.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/World/EntityComponentSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/Dummy/DummySystem.h>

using namespace Horizon;

int main(int argc, char* argv[])
{
	Engine engine;

	engine.AddContext<CacheContext>();
	engine.AddContext<GfxContext>();
	engine.AddContext<SamplerContext>();
	engine.AddContext<ShaderContext>();

	engine.AddSystem<WindowSystem>();
	engine.AddSystem<EntityComponentSystem>();
	engine.AddSystem<PresentationSystem>();
	engine.AddSystem<DummySystem>();

	engine.Run();
}