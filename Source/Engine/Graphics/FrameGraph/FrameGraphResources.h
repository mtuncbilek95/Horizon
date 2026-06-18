#pragma once

#include <Engine/Graphics/FrameGraph/FrameGraphHandle.h>

#include <vector>

namespace Horizon
{
	class GfxTexture;

	class FrameGraphResources
	{
	public:
		FrameGraphResources(const std::vector<GfxTexture*>& textures) : m_textures(textures) {}

		GfxTexture* GetTexture(FGTextureHandle handle) const { return m_textures[handle.index]; }

	private:
		const std::vector<GfxTexture*>& m_textures;
	};
}