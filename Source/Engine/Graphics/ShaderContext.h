#pragma once

#include <Runtime/Graphics/RHI/Shader/GfxShader.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/ShaderCompiler/ShaderCompiler.h>

#include <Engine/Core/Context.h>

namespace Horizon
{
	struct ShaderProgram
	{
		PipelineType type;
		std::vector<GfxShader*> shaders;
		std::vector<GfxDescriptorLayout*> layouts;
		PushConstants constants;
		InputAssembler input;
		u32 colorAttachmentCount = 0;
	};

	class ShaderContext : public Context<ShaderContext>
	{
	public:
		ShaderProgram* GetProgram(PipelineType type, const std::string& name);
		ShaderProgram* GetProgram(const std::string& name);

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		std::unordered_map<std::string, std::shared_ptr<GfxShader>> m_shaders;
		std::unordered_map<std::string, std::shared_ptr<GfxDescriptorLayout>> m_descriptorLayouts;
		std::unordered_map<std::string, ShaderProgram> m_programs;
	};
}