#include "FrameGraphResources.h"

#include <Engine/FrameGraph/FrameGraph.h>

namespace Horizon
{
	FrameGraphResources::FrameGraphResources(FrameGraph& graph) : m_graph(graph)
	{}

	GfxTextureHandle FrameGraphResources::GetTexture(FGTextureHandle handle) const 
	{
		const FrameGraphTextureResource resource = m_graph.m_textureResources[handle.index()];
		return resource.physical;
	}

	GfxBufferHandle FrameGraphResources::GetBuffer(FGBufferHandle handle) const
	{
		const FrameGraphBufferResource& resource = m_graph.m_bufferResources[handle.index()];
		return resource.physical;
	}

	u32 FrameGraphResources::GetTextureShaderView(FGTextureHandle handle) const
	{
		FrameGraphTextureResource& resource = m_graph.m_textureResources[handle.index()];
		if (resource.shaderViewIndex == ~0u)
			resource.shaderViewIndex = GfxDevice::BindlessShaderView(resource.physical);

		return resource.shaderViewIndex;
	}

	u32 FrameGraphResources::GetTextureAccessView(FGTextureHandle handle) const
	{
		FrameGraphTextureResource& resource = m_graph.m_textureResources[handle.index()];
		if (resource.accessViewIndex == ~0u)
			resource.accessViewIndex = GfxDevice::BindlessAccessView(resource.physical);

		return resource.accessViewIndex;
	}

	u32 FrameGraphResources::GetBufferShaderView(FGBufferHandle handle) const
	{
		FrameGraphBufferResource& resource = m_graph.m_bufferResources[handle.index()];
		if (resource.shaderViewIndex == ~0u)
			resource.shaderViewIndex = GfxDevice::BindlessShaderView(resource.physical);

		return resource.shaderViewIndex;
	}

	u32 FrameGraphResources::GetBufferAccessView(FGBufferHandle handle) const
	{
		FrameGraphBufferResource& resource = m_graph.m_bufferResources[handle.index()];
		if (resource.accessViewIndex == ~0u)
			resource.accessViewIndex = GfxDevice::BindlessAccessView(resource.physical);

		return resource.accessViewIndex;
	}
}