#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Shader/GfxShaderStage.h>
#include <string>

namespace Horizon::RHI
{
	struct GfxShaderCompiler
	{
		static List<u8> Compile(const std::string& filePath, GfxShaderStage stage, const std::string& entryPoint, const std::string& includePath = "");
	};
}