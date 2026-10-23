#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Rendering/RenderGraph/RenderPassFrame.h>
#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class ENGINE_API RenderPass : public Reflect::Base
	{
		friend class RenderSystem;
	public:
		virtual void OnInitialize() = 0;
		virtual void OnFinalize() = 0;

		virtual void RenderFrame(const RenderPassFrame& frame) = 0;

		RHI::GfxShader* CreateShader(RHI::GfxDevice* pDevice, const std::string& filePath, RHI::GfxShaderStage stage, const std::string& entryPoint, const std::string& debugName);

	protected:
		Engine* GetEngine() const { return m_engine; }

	private:
		Engine* m_engine = nullptr;
	};
}