#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <Editor/Domain/DomainService.h>
#include <Editor/Renderer/EditorService.h>

#include "TypeManifestation.h"

using namespace Horizon;

int main(int argC, char** argV)
{
	Engine::Engine engine;

	engine.RegisterModule<Engine::WindowService>();
	engine.RegisterModule<Engine::GraphicsContext>();
	engine.RegisterModule<Editor::DomainService>();
	engine.RegisterModule<Editor::EditorService>();

	engine.Run();
}