#include "DX12Context.h"

namespace Horizon
{
	GfxTextureHandle GfxDevice::CreateTexture(const GfxTextureDesc& desc)
	{
		return GfxTextureHandle();
	}

	void GfxDevice::WriteTexture(GfxTextureHandle handle, void* pData, usize sizeInBytes, usize offset)
	{}

	void GfxDevice::DestroyTexture(GfxTextureHandle handle)
	{}
}