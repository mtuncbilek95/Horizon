#include <Engine/Core/Engine.h>

#include <Engine/Job/JobSystem.h>
#include <Engine/Command/CommandSystem.h>
#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/Asset/AssetSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/ECS/EntityComponentSystem.h>
#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Domain/DomainSystem.h>

using namespace Horizon;

i32 main(i32 argC, c8** argV)
{
	Engine engine;

	engine.AddSystem<CommandSystem>(argC, argV);
	engine.AddSystem<WindowSystem>();
	engine.AddSystem<JobSystem>();
	engine.AddSystem<GraphicsSystem>();
	engine.AddSystem<PresentationSystem>();
	engine.AddSystem<EntityComponentSystem>();
	engine.AddSystem<EditorSystem>();
	engine.AddSystem<DomainSystem>();
	engine.AddSystem<AssetSystem>();

	engine.Run();
}
