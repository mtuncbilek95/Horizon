#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Texture/GfxTextureDesc.h>

namespace Horizon::RHI
{
	class GfxTexture : public GfxObject
	{
	public:
		const GfxTextureDesc& GetDesc() const { return m_desc; }

	protected:
		GfxTextureDesc m_desc{};
	};
}
