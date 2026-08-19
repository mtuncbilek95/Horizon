#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <Editor/Domain/DomainService.h>
#include <Editor/Renderer/EditorService.h>

/*#include <Engine/Job/JobContext.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Asset/AssetSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/ECS/EntityComponentSystem.h>
#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Domain/DomainSystem.h>*/

#include "TypeManifestation.h"

using namespace Horizon;

int main(int argC, char** argV)
{
	Engine::Engine engine;

	engine.RegisterModule<Engine::WindowService>();
	engine.RegisterModule<Engine::GraphicsContext>();
	engine.RegisterModule<Editor::DomainService>();
	engine.RegisterModule<Editor::EditorService>();

/*	engine.AddContext<Engine::JobContext>();
	engine.AddContext<Engine::GraphicsContext>();
	//engine.AddContext<ProjectContext>(argC, argV);

	engine.AddSystem<Engine::WindowSystem>();
	engine.AddSystem<Engine::EntityComponentSystem>();
	// TODO: This will be argV[1] later.
	engine.AddSystem<Editor::DomainSystem>("D:/Projects/Horizon/ExampleProject");
	engine.AddSystem<Engine::AssetSystem>();
	engine.AddSystem<Editor::EditorSystem>();
	engine.AddSystem<Engine::PresentationSystem>();*/

	engine.Run();
}