#pragma once

#include <Model/CookedModel.h>

#include <filesystem>

namespace Horizon::Asset
{
	bool ImportModel(const std::filesystem::path& sourcePath, CookedModel& outModel);
}