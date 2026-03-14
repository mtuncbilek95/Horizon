#pragma once

#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

namespace Horizon
{
	struct ShaderCompiler final
	{
		static ReadArray<u32> GenerateSpirv(const std::string& source, const std::string& entryPoint, ShaderStage stage);
	};
}
