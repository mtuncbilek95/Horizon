#include "LightCullingPass.h"

namespace Horizon::Rendering::Pass
{
	b8 LightCullingPass::OnInitialize()
	{
		return true;
	}

	void LightCullingPass::OnFinalize()
	{
	}

	void LightCullingPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
	}

	void LightCullingPass::Render(const Engine::Blackboard& board)
	{
	}
}