#include <Engine/Engine/Engine.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/ECS/ECSSystem.h>
#include <Engine/Game/GameSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/Asset/AssetSystem.h>

#include <Runtime/Data/DataReader.h>
#include <Runtime/ShaderCompiler/ShaderCompiler.h>

int main(int argc, char* argv[])
{
	Horizon::Engine newEngine;

	newEngine.AddSystem<Horizon::WindowSystem>();
	newEngine.AddSystem<Horizon::ECSSystem>();
	newEngine.AddSystem<Horizon::GameSystem>();
	newEngine.AddSystem<Horizon::GraphicsSystem>();
	newEngine.AddSystem<Horizon::PresentationSystem>();
	newEngine.AddSystem<Horizon::AssetSystem>();

	newEngine.Run();

    return 0;
}
