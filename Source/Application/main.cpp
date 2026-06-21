#include <Engine/Core/Engine.h>

#include <Engine/Job/JobModule.h>
#include <Engine/Window/WindowModule.h>
#include <Editor/Domain/DomainModule.h>
#include <Engine/Asset/AssetModule.h>
#include <Engine/Graphics/GraphicsModule.h>
#include <Editor/Editor/EditorModule.h>

using namespace Horizon;

i32 main(i32 argC, c8** argV)
{
	Engine engine;

	engine.AddModule<JobModule>();
	engine.AddModule<WindowModule>();
	engine.AddModule<DomainModule>();
	engine.AddModule<AssetModule>();
	engine.AddModule<GraphicsModule>();
	engine.AddModule<EditorModule>();

	engine.Run();
}
