#pragma once

#include <Application/GpuShared.h>

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <glm/glm.hpp>

namespace Horizon
{
	struct CameraComp;

	class Renderer
	{
	public:
		void Initialize(u32 framesInFlight);
		void Shutdown() const;

		void Render(const CameraComp& camera, GfxTextureHandle backBuffer,
			u32 width, u32 height, u32 frameSlot);

	private:
		void CreatePipelines();
		void BuildFrameGraph(GfxTextureHandle backBuffer, u32 width, u32 height, u32 frameSlot);

		GfxPipelineHandle m_shadowPipeline;
		GfxPipelineHandle m_gbufferPipeline;
		GfxPipelineHandle m_lightPipeline;
		GfxPipelineHandle m_skyPipeline;
		GfxPipelineHandle m_bloomExtractPipeline;
		GfxPipelineHandle m_bloomBlurPipeline;
		GfxPipelineHandle m_finalPipeline;

		GfxBufferHandle m_frameBuffer;
		u32 m_frameBufferShaderView = ~0u;
		u32 m_framesInFlight = 2;

		glm::vec3 m_lightDirection = glm::normalize(glm::vec3(-0.15f, -0.98f, -0.10f));
		glm::vec4 m_lightColor = glm::vec4(1.0f, 0.95f, 0.9f, 3.0f);
		f32 m_bloomThreshold = 1.0f;
	};
}