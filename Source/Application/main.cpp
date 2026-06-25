#include <Engine/Core/Engine.h>

#include <Engine/Job/JobSubsystem.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>

using namespace Horizon;

i32 main(i32 argC, c8** argV)
{
	Engine engine;

	engine.AddModule<WindowSubsystem>();
	engine.AddModule<JobSubsystem>();
	engine.AddModule<GraphicsSubsystem>();

	engine.Run();
}
