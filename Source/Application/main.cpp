#include <Engine/Core/Engine.h>

#include <Engine/Window/WindowModule.h>

using namespace Horizon;

int main(int argC, char** argV)
{
	Engine engine;

	engine.AddModule<WindowModule>();

	engine.Run();
}
