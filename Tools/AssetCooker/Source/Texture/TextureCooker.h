#pragma once

#include <Model/CookedModel.h>
#include <filesystem>

namespace Horizon::Asset
{
	bool CookTextures(CookedModel& model, const std::filesystem::path& baseDir);
}