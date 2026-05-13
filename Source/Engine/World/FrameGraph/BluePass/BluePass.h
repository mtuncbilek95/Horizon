#pragma once

#include <Engine/World/FrameGraph/FramePass.h>
#include <Engine/World/FrameGraph/ResourceHandle.h>

namespace Horizon
{
	class GfxPipeline;

	class BluePass : public FramePass
	{
	public:
		void Setup(GraphBuilder& builder) override;
		void Execute(GfxCommandBuffer* cmd, const PassResources& resources) override;

	private:
		RenderTargetHandle m_colorHandle;
		RenderTargetHandle m_depthHandle;

		GfxPipeline* m_testPipeline;
	};
}