#pragma once

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	struct TextureRecord
	{
		GfxTexture* pTex = nullptr;
		GfxTextureDesc desc;
		GfxResourceState currState = GfxResourceState::Common;
		u32 shaderView = ~0u, accessView = ~0u, colorView = ~0u, depthView = ~0u;
	};

	struct BufferRecord
	{
		GfxBuffer* pBuf = nullptr;
		GfxBufferDesc desc;
		GfxResourceState currState = GfxResourceState::Common;
		u32 shaderView = ~0u, accessView = ~0u;
	};

	struct PipelineRecord
	{
		GfxPipeline* pPipe = nullptr;
		
	};
}