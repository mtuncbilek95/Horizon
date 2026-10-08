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

#include "TypeManifestation.h"

using namespace Horizon;

int main(int argC, char** argV)
{
	std::string projectPath;

	if (argC < 2)
		projectPath = std::string(HORIZON_EXAMPLE_DIR) + "/ExampleProject.hproject";
	else
		projectPath = argV[1];

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