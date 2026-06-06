#pragma once

#include <Model/CookedModel.h>
#include <filesystem>

namespace Horizon::Asset
{
	bool WriteModel(const std::filesystem::path& outputPath, const CookedModel& model);
}