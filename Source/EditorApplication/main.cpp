#include <Editor/Domain/DomainService.h>
#include <Editor/Renderer/EditorService.h>
#include <Editor/Importer/ImporterContext.h>
#include <Editor/Project/ProjectContext.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/World/WorldService.h>
#include <Engine/Physics/PhysicsService.h>

#include <Runtime/PAL/Console/Console.h>

#include "TypeManifestation.h"

using namespace Horizon;

int main(int argC, char** argV)
{
	std::string projectPath;
	if (argC <= 1)
	{
		Terminal::Warn("Main", "Invoking the engine without a project");
		projectPath = HORIZON_EXAMPLE_DIR + std::string("/ExampleProject.hproject");
	}
	else
	{
		projectPath = argV[1];
		Terminal::Warn("Main", "Invoking the engine with project -> {}", projectPath);
	}

	if (projectPath.substr(projectPath.rfind(".")) != ".hproject")
	{
		Terminal::Error("Main", "It's not a Horizon Project!");
		return -1;
	}
	else
		Terminal::Info("Main", "{} has been registered. Opening the project!", projectPath);

	Engine::Engine engine;
	engine.RegisterModule<Editor::ProjectContext>(projectPath, HORIZON_RESOURCE_DIR);

	Engine::WindowParams windowParams =
	{
		.windowSize = Math::Vec2u::Zero(), // This will be filled by argV for force-arguments thingy.
		.flags = PAL::WindowFlags::EnableDragDrop | PAL::WindowFlags::CustomTitleBar
	};
	engine.RegisterModule<Engine::WindowService>(windowParams);

	engine.RegisterModule<Engine::GraphicsContext>();
	engine.RegisterModule<Editor::ImporterContext>();
	engine.RegisterModule<Editor::DomainService>();
	engine.RegisterModule<Engine::AssetService>();
	engine.RegisterModule<Engine::WorldService>();
	engine.RegisterModule<Editor::EditorService>();
	engine.RegisterModule<Engine::PhysicsService>();

	engine.Run();
}