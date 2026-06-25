#include <Engine/Core/Engine.h>

#include <Engine/Job/JobSubsystem.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>

using namespace Horizon;

i32 main(i32 argC, c8** argV)
{
	Engine engine;

	engine.AddSubsystem<WindowSubsystem>();
	engine.AddSubsystem<JobSubsystem>();
	engine.AddSubsystem<GraphicsSubsystem>();

	engine.Run();
}
