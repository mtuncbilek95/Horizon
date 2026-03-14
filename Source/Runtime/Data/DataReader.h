#pragma once

#include <string>
#include <filesystem>

namespace Horizon
{
	struct DataReader
	{
		static std::string ReadToTextFile(const std::filesystem::path& path);
	};
}