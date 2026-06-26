#include <Engine/Core/Engine.h>

#include <Engine/Job/JobSubsystem.h>
#include <Engine/Command/CommandSubsystem.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>
#include <Engine/Asset/AssetSubsystem.h>
#include <Engine/Presentation/PresentationSubsystem.h>
#include <Editor/Renderer/EditorSubsystem.h>
#include <Editor/Domain/DomainSubsystem.h>

using namespace Horizon;

i32 main(i32 argC, c8** argV)
{
	Engine engine;

	engine.AddSubsystem<CommandSubsystem>(argC, argV);
	engine.AddSubsystem<WindowSubsystem>();
	engine.AddSubsystem<JobSubsystem>();
	engine.AddSubsystem<GraphicsSubsystem>();
	engine.AddSubsystem<PresentationSubsystem>();
	engine.AddSubsystem<EditorSubsystem>();
	engine.AddSubsystem<DomainSubsystem>();
	engine.AddSubsystem<AssetSubsystem>();

	engine.Run();
}
