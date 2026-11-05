#include "LightCullingPass.h"

#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

namespace Horizon::Rendering::Pass
{
	b8 LightCullingPass::OnInitialize()
	{
		return true;
	}

	void LightCullingPass::OnFinalize()
	{
	}

	void LightCullingPass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		RHI::BufferDesc gridDesc = {};
		gridDesc.size = 120 * 68 * sizeof(u32) * 2;
		gridDesc.usage = RHI::BufferUsage::Storage;
		gridDesc.memory = RHI::MemoryType::GpuOnly;

		graph.Reads<Resource::SceneDepth>(RHI::ResourceState::ShaderResource);

		graph.Creates<Resource::LightGrid>(gridDesc);
		graph.Writes<Resource::LightGrid>(RHI::ResourceState::UnorderedAccess);
	}

	void LightCullingPass::Render(const Engine::RenderDrawContext& context)
	{
	}
}