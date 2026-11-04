#include "DepthPrePass.h"

#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

namespace Horizon::Rendering::Pass
{
	b8 DepthPrePass::OnInitialize()
	{
		return true;
	}

	void DepthPrePass::OnFinalize()
	{
	}

	void DepthPrePass::Setup(Engine::RenderDependency& graph)
	{
		RHI::TextureDesc depthDesc = {};
		depthDesc.format = RHI::TextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::TextureUsage::DepthStencil | RHI::TextureUsage::Sampled;
		depthDesc.width = 1920;
		depthDesc.height = 1080;

		graph.Creates<Resource::SceneDepth>(depthDesc);
		graph.Writes<Resource::SceneDepth>(RHI::ResourceState::DepthWrite);
	}

	void DepthPrePass::Render()
	{
	}
}