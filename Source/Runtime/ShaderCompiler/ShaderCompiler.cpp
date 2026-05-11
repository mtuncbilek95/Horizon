#include "ShaderCompiler.h"

#include <Runtime/Data/DataReader.h>

#include <magic_enum/magic_enum.hpp>
#include <shaderc/shaderc.hpp>

#include <spirv_reflect.h>

#include <unordered_map>
#include <vector>
#include <print>

namespace Horizon
{
	static shaderc_shader_kind ToShadercKind(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:   return shaderc_vertex_shader;
		case ShaderStage::Fragment: return shaderc_fragment_shader;
		case ShaderStage::Compute:  return shaderc_compute_shader;
		case ShaderStage::Geometry: return shaderc_geometry_shader;
		case ShaderStage::TessControl: return shaderc_tess_control_shader;
		case ShaderStage::TessEval: return shaderc_tess_evaluation_shader;
		case ShaderStage::Raygen: return shaderc_raygen_shader;
		case ShaderStage::Anyhit: return shaderc_anyhit_shader;
		case ShaderStage::ClosestHit: return shaderc_closesthit_shader;
		case ShaderStage::Miss: return shaderc_miss_shader;
		case ShaderStage::Intersection: return shaderc_intersection_shader;
		case ShaderStage::Callable: return shaderc_callable_shader;
		case ShaderStage::Task: return shaderc_task_shader;
		case ShaderStage::Mesh: return shaderc_mesh_shader;
		default:
			Log::Terminal(LogType::Fatal, "ShaderCompiler", "Unsupported ShaderStage: {}", magic_enum::enum_name(stage));
			return shaderc_vertex_shader;
		}
	}

	static ShaderDescriptorType ToShaderDescriptorType(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return ShaderDescriptorType::UniformBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return ShaderDescriptorType::StorageBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			return ShaderDescriptorType::CombinedImageSampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return ShaderDescriptorType::StorageImage;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return ShaderDescriptorType::SampledImage;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
			return ShaderDescriptorType::Sampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			return ShaderDescriptorType::InputAttachment;
		default:
			return ShaderDescriptorType::UniformBuffer;
		}
	}

	static ShaderDataFormat ToShaderDataFormatFromSpv(SpvReflectFormat format)
	{
		switch (format)
		{
		case SPV_REFLECT_FORMAT_R32_SFLOAT:
			return ShaderDataFormat::Float;
		case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
			return ShaderDataFormat::Vec2;
		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
			return ShaderDataFormat::Vec3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
			return ShaderDataFormat::Vec4;
		case SPV_REFLECT_FORMAT_R32_SINT:
			return ShaderDataFormat::Int;
		case SPV_REFLECT_FORMAT_R32G32_SINT:
			return ShaderDataFormat::IVec2;
		case SPV_REFLECT_FORMAT_R32G32B32_SINT:
			return ShaderDataFormat::IVec3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
			return ShaderDataFormat::IVec4;
		case SPV_REFLECT_FORMAT_R32_UINT:
			return ShaderDataFormat::UInt;
		case SPV_REFLECT_FORMAT_R32G32_UINT:
			return ShaderDataFormat::UVec2;
		case SPV_REFLECT_FORMAT_R32G32B32_UINT:
			return ShaderDataFormat::UVec3;
		case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
			return ShaderDataFormat::UVec4;
		default:
			return ShaderDataFormat::Float;
		}
	}

	static ShaderDataFormat ToShaderDataFormat(const SpvReflectNumericTraits& numeric, const SpvReflectTypeDescription& traits)
	{
		if (numeric.matrix.column_count > 0)
		{
			if (numeric.matrix.column_count == 3)
				return ShaderDataFormat::Mat3;
			if (numeric.matrix.column_count == 4)
				return ShaderDataFormat::Mat4;
		}

		u32 compCount = numeric.vector.component_count;
		b8 isFloat = (numeric.scalar.width == 32 && numeric.scalar.signedness == 0);
		b8 isSigned = (numeric.scalar.signedness == 1);

		if (compCount == 0 || compCount == 1)
		{
			if (isSigned)
				return ShaderDataFormat::Int;
			if (isFloat)
				return ShaderDataFormat::Float;
			return ShaderDataFormat::UInt;
		}

		if (isFloat)
		{
			if (compCount == 2)
				return ShaderDataFormat::Vec2;
			if (compCount == 3)
				return ShaderDataFormat::Vec3;
			if (compCount == 4)
				return ShaderDataFormat::Vec4;
		}

		if (isSigned)
		{
			if (compCount == 2)
				return ShaderDataFormat::IVec2;
			if (compCount == 3)
				return ShaderDataFormat::IVec3;
			if (compCount == 4)
				return ShaderDataFormat::IVec4;
		}

		if (compCount == 2)
			return ShaderDataFormat::UVec2;
		if (compCount == 3)
			return ShaderDataFormat::UVec3;
		if (compCount == 4)
			return ShaderDataFormat::UVec4;

		return ShaderDataFormat::Float;
	}

	ReadArray<u32> ShaderCompiler::GenerateSpirv(const std::string& source, const std::string& entryPoint, ShaderStage stage)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
#if defined(HORIZON_DEBUG)
		options.SetGenerateDebugInfo();
#endif

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
			source,
			ToShadercKind(stage),
			entryPoint.c_str(),
			options
		);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			Log::Terminal(LogType::Fatal, "ShaderCompiler", "SPIR-V compilation failed: {}", result.GetErrorMessage());
		}

		std::vector<u32> spirv(result.cbegin(), result.cend());
		return ReadArray<u32>(spirv.data(), static_cast<u32>(spirv.size()));
	}

	ShaderData ShaderCompiler::ReadShaderData(const std::filesystem::path& path)
	{
		static const std::unordered_map<std::string, ShaderStage> s_extToStage =
		{
			{ ".vert", ShaderStage::Vertex },
			{ ".frag", ShaderStage::Fragment },
			{ ".comp", ShaderStage::Compute },
			{ ".geom", ShaderStage::Geometry },
			{ ".tesc", ShaderStage::TessControl },
			{ ".tese", ShaderStage::TessEval },
			{ ".mesh", ShaderStage::Mesh },
			{ ".task", ShaderStage::Task },
			{ ".rgen", ShaderStage::Raygen },
			{ ".rahit", ShaderStage::Anyhit },
			{ ".rchit", ShaderStage::ClosestHit },
			{ ".rmiss", ShaderStage::Miss },
			{ ".rint", ShaderStage::Intersection },
			{ ".rcall", ShaderStage::Callable },
		};

		std::string ext = path.extension().string();
		auto it = s_extToStage.find(ext);
		if (it == s_extToStage.end())
		{
			Log::Terminal(LogType::Fatal, "ShaderCompiler", "Unknown shader extension: {}", ext);
			return ShaderData();
		}

		ShaderData data;
		data.sourceText = DataReader::ReadToTextFile(path);
		data.entryPoint = "main";
		data.stage = it->second;
		return data;
	}

	ShaderReflectionData ShaderCompiler::ReflectShader(const ReadArray<u32>& spirv, const ShaderData& dataInfo)
	{
		ShaderReflectionData reflectionData = {};
		reflectionData.stage = dataInfo.stage;
		reflectionData.entryPoint = dataInfo.entryPoint;

		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(
			spirv.Size() * sizeof(u32), spirv.Data(), &module);

		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			Log::Terminal(LogType::Fatal, "ShaderCompiler",
				"SPIR-V reflection failed: {}", magic_enum::enum_name(result));
			return reflectionData;
		}

		{
			u32 count = 0;
			spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);

			std::vector<SpvReflectDescriptorBinding*> bindings(count);
			spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

			for (auto* b : bindings)
			{
				DescriptorBindingInfo info = {};
				info.name = b->name ? b->name : "";
				info.set = b->set;
				info.binding = b->binding;
				info.type = ToShaderDescriptorType(b->descriptor_type);

				info.count = 1;
				for (u32 d = 0; d < b->array.dims_count; d++)
					info.count *= b->array.dims[d];

				if (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
					b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				{
					for (u32 i = 0; i < b->block.member_count; i++)
					{
						auto& m = b->block.members[i];
						ShaderBufferMember member = {};
						member.name = m.name ? m.name : "";
						member.offset = m.offset;
						member.size = m.size;
						member.format = ToShaderDataFormat(m.numeric, *m.type_description);
						info.members.push_back(std::move(member));
					}
				}

				reflectionData.descriptors.push_back(std::move(info));
			}
		}

		{
			u32 count = 0;
			spvReflectEnumeratePushConstantBlocks(&module, &count, nullptr);

			std::vector<SpvReflectBlockVariable*> blocks(count);
			spvReflectEnumeratePushConstantBlocks(&module, &count, blocks.data());

			for (auto* pc : blocks)
			{
				PushConstantInfo info = {};
				info.offset = pc->offset;
				info.size = pc->size;

				for (u32 i = 0; i < pc->member_count; i++)
				{
					auto& m = pc->members[i];
					ShaderBufferMember member = {};
					member.name = m.name ? m.name : "";
					member.offset = m.offset;
					member.size = m.size;
					member.format = ToShaderDataFormat(m.numeric, *m.type_description);
					info.members.push_back(std::move(member));
				}

				reflectionData.pushConstants.push_back(std::move(info));
			}
		}

		{
			u32 count = 0;
			spvReflectEnumerateInputVariables(&module, &count, nullptr);

			std::vector<SpvReflectInterfaceVariable*> inputs(count);
			spvReflectEnumerateInputVariables(&module, &count, inputs.data());

			for (auto* var : inputs)
			{
				if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
					continue;

				ShaderIOVariable io = {};
				io.name = var->name ? var->name : "";
				io.location = var->location;
				io.format = ToShaderDataFormatFromSpv(var->format);
				reflectionData.inputs.push_back(std::move(io));
			}
		}

		{
			u32 count = 0;
			spvReflectEnumerateOutputVariables(&module, &count, nullptr);

			std::vector<SpvReflectInterfaceVariable*> outputs(count);
			spvReflectEnumerateOutputVariables(&module, &count, outputs.data());

			for (auto* var : outputs)
			{
				if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
					continue;

				ShaderIOVariable io = {};
				io.name = var->name ? var->name : "";
				io.location = var->location;
				io.format = ToShaderDataFormatFromSpv(var->format);
				reflectionData.outputs.push_back(std::move(io));
			}
		}

		spvReflectDestroyShaderModule(&module);
		return reflectionData;
	}

	PipelineReflectionData ShaderCompiler::MergeReflections(std::span<const ShaderReflectionData> shaders)
	{
		PipelineReflectionData pipeline = {};
		std::map<u32, std::map<u32, MergedBindingInfo>> setMap;

		for (auto& shader : shaders)
		{
			for (auto& desc : shader.descriptors)
			{
				auto& binding = setMap[desc.set][desc.binding];

				if (binding.stages == static_cast<ShaderStage>(0))
				{
					binding.name = desc.name;
					binding.binding = desc.binding;
					binding.count = desc.count;
					binding.type = desc.type;
					binding.members = desc.members;
				}

				binding.stages |= shader.stage;
			}

			for (auto& pc : shader.pushConstants)
			{
				auto it = std::ranges::find_if(pipeline.pushConstants,
					[&](const PushConstantInfo& existing) {
						return existing.offset == pc.offset && existing.size == pc.size;
					});

				if (it != pipeline.pushConstants.end())
					it->stages |= shader.stage;
				else
				{
					PushConstantInfo merged = {};
					merged.offset = pc.offset;
					merged.size = pc.size;
					merged.stages = shader.stage;
					merged.members = pc.members;
					pipeline.pushConstants.push_back(std::move(merged));
				}
			}

			pipeline.inputs[shader.stage] = shader.inputs;
			pipeline.outputs[shader.stage] = shader.outputs;
		}

		for (auto& [setIndex, bindingMap] : setMap)
		{
			DescriptorSetInfo setInfo = {};
			setInfo.set = setIndex;

			for (auto& [_, binding] : bindingMap)
				setInfo.bindings.push_back(std::move(binding));

			pipeline.sets.push_back(std::move(setInfo));
		}

		return pipeline;
	}
}