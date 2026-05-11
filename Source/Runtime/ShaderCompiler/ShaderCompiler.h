#pragma once

#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <filesystem>
#include <map>

namespace Horizon
{
	struct ShaderData
	{
		std::string sourceText;
		std::string entryPoint;
		ShaderStage stage;
	};

	enum class ShaderDescriptorType
	{
		UniformBuffer,
		StorageBuffer,
		CombinedImageSampler,
		StorageImage,
		SampledImage,
		Sampler,
		InputAttachment
	};

	enum class ShaderDataFormat : u8
	{
		Float, Vec2, Vec3, Vec4,
		Int, IVec2, IVec3, IVec4,
		UInt, UVec2, UVec3, UVec4,
		Mat3, Mat4
	};

	struct ShaderBufferMember
	{
		std::string name;
		u32 offset;
		u32 size;
		ShaderDataFormat format;
	};

	struct ShaderIOVariable
	{
		std::string name;
		u32 location;
		ShaderDataFormat format;
	};

	struct DescriptorBindingInfo
	{
		std::string name;
		u32 set;
		u32 binding;
		u32 count;
		ShaderDescriptorType type;
		std::vector<ShaderBufferMember> members;
	};

	struct PushConstantInfo
	{
		u32 offset;
		u32 size;
		ShaderStage stages;
		std::vector<ShaderBufferMember> members;
	};

	struct ShaderReflectionData
	{
		ShaderStage stage;
		std::string entryPoint;

		std::vector<DescriptorBindingInfo> descriptors;
		std::vector<PushConstantInfo> pushConstants;
		std::vector<ShaderIOVariable> inputs;
		std::vector<ShaderIOVariable> outputs;
	};

	struct MergedBindingInfo
	{
		std::string name;
		u32 binding;
		u32 count;
		ShaderDescriptorType type;
		ShaderStage stages;
		std::vector<ShaderBufferMember> members;
	};

	struct DescriptorSetInfo
	{
		u32 set;
		std::vector<MergedBindingInfo> bindings;
	};

	struct PipelineReflectionData
	{
		std::vector<DescriptorSetInfo> sets;
		std::vector<PushConstantInfo> pushConstants;
		std::map<ShaderStage, std::vector<ShaderIOVariable>> inputs;
		std::map<ShaderStage, std::vector<ShaderIOVariable>> outputs;
	};

	struct ShaderCompiler final
	{
		static ReadArray<u32> GenerateSpirv(const std::string& source, const std::string& entryPoint, ShaderStage stage);
		static ShaderData ReadShaderData(const std::filesystem::path& path);
		static ShaderReflectionData ReflectShader(const ReadArray<u32>& spirv, const ShaderData& dataInfo);
		static PipelineReflectionData MergeReflections(std::span<const ShaderReflectionData> shaders);
	};
}