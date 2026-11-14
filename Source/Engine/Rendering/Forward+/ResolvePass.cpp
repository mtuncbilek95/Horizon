#include "ResolvePass.h"

#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/Renderables/RenderableArea.h>

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

	void ResolvePass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc resolvedDesc = {};
		resolvedDesc.format = Engine::PresentationService::SceneFormat;
		resolvedDesc.usage = RHI::TextureUsage::Sampled;
		resolvedDesc.width = pArea->colorImageSize.X();
		resolvedDesc.height = pArea->colorImageSize.Y();

		graph.Creates<Resource::SceneColorResolved>(resolvedDesc);
		graph.Reads<Resource::SceneColorMS>(RHI::ResourceState::ResolveSrc);
		graph.Writes<Resource::SceneColorResolved>(RHI::ResourceState::ResolveDst);
	}

	void ResolvePass::Render(const Engine::Blackboard& board)
	{
		RHI::CommandList* pCmdList = GetCommandList();

		pCmdList->ResolveTexture(GetTexture<Resource::SceneColorMS>(), GetTexture<Resource::SceneColorResolved>());
	}
}
