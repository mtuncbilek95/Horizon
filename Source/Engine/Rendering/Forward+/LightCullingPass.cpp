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
	}

	void LightCullingPass::Render(const Engine::RenderDrawContext& context)
	{
	}
}