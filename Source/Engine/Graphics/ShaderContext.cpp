#include "ShaderContext.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

#include <Engine/Graphics/GfxContext.h>

#include <print>
#include <filesystem>
namespace fs = std::filesystem;

namespace Horizon
{
	EngineReport ShaderContext::OnInitialize()
	{
		auto& gDevice = RequestContext<GfxContext>().Device();

		std::map<std::string, std::vector<ShaderReflectionData>> graphicsGroups;
		std::map<std::string, std::vector<ShaderReflectionData>> computeGroups;
		std::map<std::string, std::vector<ShaderReflectionData>> raytracingGroups;

		for (const auto& entry : fs::recursive_directory_iterator("Shaders/"))
		{
			ShaderData data = ShaderCompiler::ReadShaderData(entry.path());
			ReadArray<u32> spirv = ShaderCompiler::GenerateSpirv(data.sourceText, data.entryPoint, data.stage);
			ShaderReflectionData reflectedData = ShaderCompiler::ReflectShader(spirv, data);

			std::string name = entry.path().stem().string();
			std::string key = name + "::" + entry.path().extension().string().substr(1);

			switch (reflectedData.stage)
			{
			case ShaderStage::Compute:
				computeGroups[name].push_back(std::move(reflectedData));
				break;
			case ShaderStage::Raygen:
			case ShaderStage::Miss:
			case ShaderStage::ClosestHit:
			case ShaderStage::Anyhit:
			case ShaderStage::Intersection:
			case ShaderStage::Callable:
				raytracingGroups[name].push_back(std::move(reflectedData));
				break;
			default:
				graphicsGroups[name].push_back(std::move(reflectedData));
				break;
			}

			m_shaders[key] = gDevice.CreateShader(GfxShaderDesc()
				.setStage(data.stage)
				.setByteCode(*spirv));
			Log::Terminal(LogType::Debug, "ShaderContext", "{} has been cached.", key);
		}

		for (auto& [name, shaders] : graphicsGroups)
		{
			PipelineReflectionData reflection = ShaderCompiler::MergeReflections(shaders);

		}

		return EngineReport();
	}

	void ShaderContext::OnFinalize()
	{
	}
}