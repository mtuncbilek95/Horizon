#include "ShaderContext.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

#include <Engine/Graphics/GfxContext.h>

#include <magic_enum/magic_enum.hpp>
#include <print>
#include <filesystem>
namespace fs = std::filesystem;

namespace Horizon
{
	static std::string StageToExtension(ShaderStage stg)
	{
		switch (stg)
		{
		case ShaderStage::Vertex:
			return "vert";
		case ShaderStage::TessControl:
			return "tesc";
		case ShaderStage::TessEval:
			return "tese";
		case ShaderStage::Geometry:
			return "geom";
		case ShaderStage::Fragment:
			return "frag";
		case ShaderStage::Compute:
			return "comp";
		case ShaderStage::Raygen:
			return "rgen";
		case ShaderStage::Anyhit:
			return "rahit";
		case ShaderStage::ClosestHit:
			return "rchit";
		case ShaderStage::Miss:
			return "rmiss";
		case ShaderStage::Intersection:
			return "rint";
		case ShaderStage::Callable:
			return "rcall";
		case ShaderStage::Task:
			return "task";
		case ShaderStage::Mesh:
			return "mesh";
		default:
			return "unknown";
		}
	}

	ShaderProgram* ShaderContext::GetProgram(PipelineType type, const std::string& name)
	{
		std::string keyType = magic_enum::enum_name<PipelineType>(type).data();
		std::string pKey = name + "::" + keyType;

		auto it = m_programs.find(pKey);
		if (it == m_programs.end())
		{
			Log::Terminal(LogType::Error, "ShaderContext", "{} has not been found. Horizon will crash!!!", pKey);
			return nullptr;
		}

		return &it->second;
	}

	ShaderProgram* ShaderContext::GetProgram(const std::string& name)
	{
		auto it = m_programs.find(name);
		if (it == m_programs.end())
		{
			Log::Terminal(LogType::Error, "ShaderContext", "{} has not been found. Horizon will crash!!!", name);
			return nullptr;
		}

		return &it->second;
	}

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

		// Graphics Programs
		for (auto& [name, shaders] : graphicsGroups)
		{
			PipelineReflectionData reflection = ShaderCompiler::MergeReflections(shaders);

			ShaderProgram program = {};

			for (auto& shader : shaders)
			{
				std::string key = name + "::" + StageToExtension(shader.stage);
				program.shaders.push_back(m_shaders[key].get());
			}

			for (auto& set : reflection.sets)
			{
				GfxDescriptorLayoutDesc layoutDesc = {};

				for (auto& binding : set.bindings)
				{
					layoutDesc.addBinding(DescriptorBinding()
						.setBinding(binding.binding)
						.setCount(binding.count)
						.setStage(binding.stages)
						.setType(binding.type));
				}

				layoutDesc.setFlags(DescriptorLayoutFlags::Buffer);

				std::string key = name + "::set" + std::to_string(set.set);
				m_descriptorLayouts[key] = gDevice.CreateDescriptorLayout(layoutDesc);
				program.layouts.push_back(m_descriptorLayouts[key].get());
			}

			for (auto& pc : reflection.pushConstants)
			{
				program.constants.addRange(PushConstantRange()
					.setStage(pc.stages)
					.setOffset(pc.offset)
					.setSizeOfData(pc.size));
			}

			auto vertIt = reflection.inputs.find(ShaderStage::Vertex);
			if (vertIt != reflection.inputs.end())
			{
				InputBinding binding = {};
				binding.setInputRate(VertexInputRate::Vertex);

				for (auto& attr : vertIt->second)
					binding.addAttribute(attr.format);

				program.input
					.setTopology(PrimitiveTopology::TriangleList)
					.addBinding(binding);
			}

			auto fragIt = reflection.outputs.find(ShaderStage::Fragment);
			program.colorAttachmentCount = fragIt != reflection.outputs.end()
				? static_cast<u32>(fragIt->second.size()) : 0;

			std::string keyType = magic_enum::enum_name<PipelineType>(PipelineType::Graphics).data();
			std::string pKey = name + "::" + keyType;
			m_programs[pKey] = std::move(program);
			Log::Terminal(LogType::Debug, "ShaderContext", "Program '{}' created.", pKey);
		}

		// Compute Programs
		for (auto& [name, shaders] : computeGroups)
		{
			PipelineReflectionData reflection = ShaderCompiler::MergeReflections(shaders);

			ShaderProgram program = {};

			std::string key = name + "::comp";
			program.shaders.push_back(m_shaders[key].get());

			for (auto& set : reflection.sets)
			{
				GfxDescriptorLayoutDesc layoutDesc = {};

				for (auto& binding : set.bindings)
				{
					layoutDesc.addBinding(DescriptorBinding()
						.setBinding(binding.binding)
						.setCount(binding.count)
						.setStage(binding.stages)
						.setType(binding.type));
				}

				layoutDesc.setFlags(DescriptorLayoutFlags::Buffer);

				std::string key = name + "::set" + std::to_string(set.set);
				m_descriptorLayouts[key] = gDevice.CreateDescriptorLayout(layoutDesc);
				program.layouts.push_back(m_descriptorLayouts[key].get());
			}

			for (auto& pc : reflection.pushConstants)
			{
				program.constants.addRange(PushConstantRange()
					.setStage(pc.stages)
					.setOffset(pc.offset)
					.setSizeOfData(pc.size));
			}

			std::string keyType = magic_enum::enum_name<PipelineType>(PipelineType::Compute).data();
			std::string pKey = name + "::" + keyType;
			m_programs[pKey] = std::move(program);
			Log::Terminal(LogType::Debug, "ShaderContext", "Compute program '{}' created.", pKey);
		}

		return EngineReport();
	}


	void ShaderContext::OnFinalize()
	{
		m_programs.clear();
		m_descriptorLayouts.clear();
		m_shaders.clear();
	}
}