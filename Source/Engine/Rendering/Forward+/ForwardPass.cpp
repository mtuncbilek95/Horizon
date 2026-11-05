#include "ForwardPass.h"

#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Texture/Texture.h>

namespace Horizon::Rendering::Pass
{
	b8 ForwardPass::OnInitialize()
	{
		return true;
	}

	void ForwardPass::OnFinalize()
	{
	}

	void ForwardPass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		graph.Reads<Resource::SceneDepth>(RHI::ResourceState::DepthWrite);
		graph.Reads<Resource::LightGrid>(RHI::ResourceState::ShaderResource);
		graph.Writes<Resource::SceneColor>(RHI::ResourceState::RenderTarget);
	}

	void ForwardPass::Render(const Engine::RenderDrawContext& context)
	{
		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColor>();

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColor>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachments[0].clearColor = { 0.1f, 0.1f, 0.1f, 1.f };
		desc.colorAttachmentCount = 1;

		desc.depthAttachment.pView = GetTextureView<Resource::SceneDepth>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Load;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.readOnly = true;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pColor->GetWidth());
		viewport.height = static_cast<f32>(pColor->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pColor->GetWidth());
		scissor.height = static_cast<i32>(pColor->GetHeight());
		pCmdList->SetScissor(scissor);

		pCmdList->EndRendering();
	}
}