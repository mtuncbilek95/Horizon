#include "RenderPass.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>

namespace Horizon::Engine
{
	UniquePtr<RHI::ShaderModule> RenderPass::CreateShader(RHI::Device* pDevice, const std::string& filePath, RHI::ShaderStage stage, const std::string& entryPoint, const std::string& debugName)
	{
		RHI::ShaderCompileDesc compileDesc = {};
		compileDesc.filePath = filePath;
		compileDesc.entryPoint = entryPoint;
		compileDesc.stage = stage;
		compileDesc.target = RHI::ShaderTarget::DXIL;

		List<u8> byteCode = RHI::ShaderCompiler::Compile(compileDesc);

		if (byteCode.IsEmpty())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be compiled with entry point {}", filePath, entryPoint);
			return nullptr;
		}

		RHI::ShaderModuleDesc moduleDesc = {};
		moduleDesc.stage = stage;
		moduleDesc.entryPoint = entryPoint;
		moduleDesc.byteCode = byteCode;

		UniquePtr<RHI::ShaderModule> pShader = pDevice->CreateRHIShaderModule(moduleDesc);
		if (!pShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be turned into a shader object", filePath);
			return nullptr;
		}

		pShader->SetDebugName(debugName.data());

		return pShader;
	}
}
