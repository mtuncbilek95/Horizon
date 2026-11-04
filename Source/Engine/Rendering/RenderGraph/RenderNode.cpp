#include "RenderNode.h"

#include <Engine/Rendering/RenderGraph/RenderGraph.h>
#include <Engine/Rendering/RenderGraph/RenderResource.h>

namespace Horizon::Engine
{
	RHI::Texture* RenderNode::FindTexture(Reflect::TypeHandle resource) const
	{
		RenderResource* pResource = m_graph->LookupResource(resource, RenderResourceType::Texture);
		if (!pResource)
			return nullptr;

		return pResource->texture.Get();
	}

	RHI::TextureView* RenderNode::FindTextureView(Reflect::TypeHandle resource) const
	{
		RenderResource* pResource = m_graph->LookupResource(resource, RenderResourceType::Texture);
		if (!pResource)
			return nullptr;

		return pResource->texView.Get();
	}

	RHI::Buffer* RenderNode::FindBuffer(Reflect::TypeHandle resource) const
	{
		RenderResource* pResource = m_graph->LookupResource(resource, RenderResourceType::Buffer);
		if (!pResource)
			return nullptr;

		return pResource->buffer.Get();
	}

}