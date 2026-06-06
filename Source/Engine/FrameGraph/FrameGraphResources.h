#pragma once

#include <Engine/FrameGraph/FrameGraphHandle.h>

namespace Horizon
{
	class FrameGraph;

	class FrameGraphResources
	{
	public:
		FrameGraphResources(FrameGraph& graph);

		GfxTextureHandle GetTexture(FGTextureHandle handle) const;
		GfxBufferHandle GetBuffer(FGBufferHandle handle) const;
		
		// Texture - Shader Resource View
		u32 GetTextureShaderView(FGTextureHandle handle) const;
		// Texture - Unordered Access View
		u32 GetTextureAccessView(FGTextureHandle handle) const;
		// Buffer - Shader Resource View
		u32 GetBufferShaderView(FGBufferHandle handle) const;
		// Buffer - Unordered Access View
		u32 GetBufferAccessView(FGBufferHandle handle) const;

	private:
		FrameGraph& m_graph;
	};
}