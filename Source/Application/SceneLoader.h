#pragma once

#include <Engine/ECS/ECS.h>

#include <filesystem>
#include <vector>

namespace Horizon
{
	EntityHandle LoadScene(std::vector<std::filesystem::path> modelPaths);
}