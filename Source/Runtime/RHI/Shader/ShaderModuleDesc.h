#pragma once

#include <Runtime/Containers/Vector/ReadOnlyList.h>
#include <Runtime/RHI/Shader/ShaderStage.h>

#include <string>

namespace Horizon::RHI
{
	struct ShaderModuleDesc final
	{
		ShaderStage stage = ShaderStage::None;
		std::string entryPoint = "main";
		ReadOnlyList<u8> byteCode;
	};
}