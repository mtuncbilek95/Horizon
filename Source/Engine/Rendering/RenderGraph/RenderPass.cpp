#include "RenderPass.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/RHI/Shader/GfxShaderCompiler.h>
#include <Runtime/RHI/Device/GfxDevice.h>

namespace Horizon::Engine
{
	RHI::GfxShader* RenderPass::CreateShader(RHI::GfxDevice* pDevice, const std::string& filePath, RHI::GfxShaderStage stage, const std::string& entryPoint, const std::string& debugName)
	{
		List<u8> byteCode = RHI::GfxShaderCompiler::Compile(filePath, stage, entryPoint);

		if (byteCode.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be compiled with entry point {}", filePath, entryPoint);
			return nullptr;
		}

		RHI::GfxShaderDesc shaderDesc = {};
		shaderDesc.stage = stage;
		shaderDesc.pByteCode = byteCode.GetData();
		shaderDesc.byteCodeSize = byteCode.GetCount();

		RHI::GfxShader* pShader = pDevice->CreateShader(shaderDesc);
		if (pShader == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be turned into a shader object", filePath);
			return nullptr;
		}

		pShader->SetDebugName(debugName.data());

		return pShader;
	}
}