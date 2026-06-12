#include <Engine/Engine/Engine.h>
#include <Engine/CommandLine/CommandLineModule.h>
#include <Engine/Job/JobModule.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Graphics/GraphicsModule.h>
#include <Engine/ECS/EntityComponentModule.h>
#include <Engine/Present/PresentModule.h>

#include <Editor/Editor/EditorModule.h>

using namespace Horizon;

int main(int argC, char** argV)
{
	Engine engine;

	engine.AddModule<CommandLineModule>(argC, argV);
	engine.AddModule<JobModule>();
	engine.AddModule<WindowModule>();
	// engine.AddModule<InputModule>();
	engine.AddModule<EntityComponentModule>();
	// engine.AddModule<GameModule>();
	engine.AddModule<GraphicsModule>();
	engine.AddModule<EditorModule>();
	engine.AddModule<PresentModule>();

	engine.Run();
}
