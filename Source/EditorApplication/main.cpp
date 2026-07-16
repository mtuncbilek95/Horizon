#include <Engine/Core/Engine.h>

#include <Engine/Job/JobContext.h>
#include <Engine/Command/CommandContext.h>
#include <Editor/Project/ProjectContext.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Asset/AssetSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/ECS/EntityComponentSystem.h>
#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Domain/DomainSystem.h>

#include "TypeManifestation.h"

using namespace Horizon;

i32 main(i32 argC, c8** argV)
{
	Engine engine;

	engine.AddContext<JobContext>();
	engine.AddContext<GraphicsContext>();
	engine.AddContext<ProjectContext>(argC, argV);

	engine.AddSystem<WindowSystem>();
	engine.AddSystem<PresentationSystem>();
	engine.AddSystem<EntityComponentSystem>();
	engine.AddSystem<EditorSystem>();
	engine.AddSystem<DomainSystem>();
	engine.AddSystem<AssetSystem>();

	engine.Run();
}