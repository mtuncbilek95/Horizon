#include "ResolvePass.h"

#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Texture/Texture.h>

namespace Horizon::Rendering::Pass
{
	b8 ResolvePass::OnInitialize()
	{
		return true;
	}

	void ResolvePass::OnFinalize()
	{
	}

	void ResolvePass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		graph.Reads<Resource::SceneColorMS>(RHI::ResourceState::ResolveSrc);
		graph.Writes<Resource::SceneColor>(RHI::ResourceState::ResolveDst);
	}

	void ResolvePass::Render(const Engine::Blackboard& board)
	{
		RHI::CommandList* pCmdList = GetCommandList();

		pCmdList->ResolveTexture(GetTexture<Resource::SceneColorMS>(), GetTexture<Resource::SceneColor>());
	}
}