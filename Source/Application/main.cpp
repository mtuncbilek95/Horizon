#include <Engine/Core/Engine.h>

#include <Engine/Window/WindowModule.h>
#include <Engine/Graphics/GraphicsModule.h>

using namespace Horizon;

int main(int argC, char** argV)
{
	Engine engine;

	engine.AddModule<WindowModule>();
	engine.AddModule<GraphicsModule>();

	engine.Run();
}
