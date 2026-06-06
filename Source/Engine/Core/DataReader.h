#pragma once

#include <fstream>
#include <vector>

namespace Horizon::DataReader
{
	static std::vector<u8> LoadFile(const char* path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		usize fileSize = (usize)file.tellg();
		std::vector<u8> data(fileSize);
		file.seekg(0);
		file.read((char*)data.data(), fileSize);
		return data;
	}
}