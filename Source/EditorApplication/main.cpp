#include <Editor/Domain/DomainService.h>
#include <Editor/Renderer/EditorService.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/World/WorldService.h>

#include "TypeManifestation.h"

using namespace Horizon;

int main(int argC, char** argV)
{
	// TODO: This will change later. If you check this and judge me, FUCK YOU MOTHERFUCKER!
	const std::string projectPath = "D:/Projects/Horizon/ExampleProject";

	Engine::Engine engine;

	engine.RegisterModule<Engine::WindowService>();
	engine.RegisterModule<Engine::GraphicsContext>();
	engine.RegisterModule<Editor::DomainService>();
	engine.RegisterModule<Engine::AssetService>();
	engine.RegisterModule<Engine::WorldService>();
	engine.RegisterModule<Editor::EditorService>();

	engine.Run();
}