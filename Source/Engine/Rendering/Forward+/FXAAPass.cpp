#include "FXAAPass.h"

namespace Horizon::Rendering::Pass
{
	b8 FXAAPass::OnInitialize()
	{
		return false;
	}

	void FXAAPass::OnFinalize()
	{
	}

	void FXAAPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
	}

	void FXAAPass::Render(const Engine::Blackboard& board)
	{
	}
}