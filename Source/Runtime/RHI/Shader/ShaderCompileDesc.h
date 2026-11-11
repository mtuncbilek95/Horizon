#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Shader/ShaderStage.h>
#include <Runtime/RHI/Shader/ShaderTarget.h>

#include <string>

namespace Horizon::RHI
{
	struct ShaderCompileDesc final
	{
		std::string filePath;
		std::string entryPoint = "main";
		ShaderStage stage = ShaderStage::None;
		ShaderTarget target = ShaderTarget::DX12;
		List<std::string> includePaths;
		List<std::string> defines;
	};
}