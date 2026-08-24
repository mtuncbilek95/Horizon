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
	Engine::Engine engine;

	engine.RegisterModule<Engine::WindowService>();
	engine.RegisterModule<Engine::GraphicsContext>();
	engine.RegisterModule<Engine::AssetService>();
	engine.RegisterModule<Editor::DomainService>();
	engine.RegisterModule<Engine::WorldService>();
	engine.RegisterModule<Editor::EditorService>();

	engine.Run();
}