#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
	DXGI_FORMAT ToDXGI(GfxTextureFormat fmt);
	b8 IsDepthFormat(GfxTextureFormat fmt);
	DXGI_FORMAT ToTypeless(GfxTextureFormat fmt);
	DXGI_FORMAT ToShaderViewFormat(GfxTextureFormat fmt);
	D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state);
}