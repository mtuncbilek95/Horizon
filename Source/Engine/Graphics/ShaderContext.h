#pragma once

#include <Runtime/Graphics/RHI/Shader/GfxShader.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/ShaderCompiler/ShaderCompiler.h>

#include <Engine/Core/Context.h>

namespace Horizon
{
	class ShaderContext : public Context<ShaderContext>
	{
	public:
		GfxShader* Get(const std::string& name);

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		std::unordered_map<std::string, std::shared_ptr<GfxShader>> m_shaders;
		std::unordered_map<std::string, PipelineReflectionData> m_pipelineReflection;
		std::unordered_map<std::string, std::shared_ptr<GfxPipeline>> m_pipelines;
	};
}