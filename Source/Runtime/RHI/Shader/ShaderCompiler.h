#pragma once

#include <Runtime/RHI/Shader/ShaderCompileDesc.h>

namespace Horizon::RHI
{
	struct ShaderCompiler
	{
		static List<u8> Compile(const ShaderCompileDesc& desc);
	};
}