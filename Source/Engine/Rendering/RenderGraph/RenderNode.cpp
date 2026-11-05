#include "RenderNode.h"

#include <Engine/Rendering/RenderGraph/RenderGraph.h>

namespace Horizon::Engine
{
	RHI::Texture* RenderNode::FindTexture(Reflect::TypeHandle resource) const
	{
		return m_graph->FindTexture(resource);
	}

	RHI::TextureView* RenderNode::FindTextureView(Reflect::TypeHandle resource) const
	{
		return m_graph->FindTextureView(resource);
	}

	RHI::Buffer* RenderNode::FindBuffer(Reflect::TypeHandle resource) const
	{
		return m_graph->FindBuffer(resource);
	}
}