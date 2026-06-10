#include <Engine/Engine/Engine.h>
#include <Engine/CommandLine/CommandLineModule.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Graphics/GraphicsModule.h>

using namespace Horizon;

int main(int argC, char** argV)
{
	Engine engine;

	engine.AddModule<CommandLineModule>(argC, argV);
	engine.AddModule<WindowModule>();
	engine.AddModule<GraphicsModule>();

	engine.Run();
}
