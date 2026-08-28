#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Texture/GfxTextureDesc.h>

namespace Horizon::RHI
{
	/**
	 * @brief Image resource covering every texture kind,
	 * from sampled material maps to render targets and
	 * depth buffers. Each usage owns its own view index.
	 *
	 * @code
	 *   RHI::GfxTextureDesc texDesc = {};
	 *   RHI::GfxTexture* tex = nullptr;
	 *   tex = myRhiDevice->CreateTexture(texDesc);
	 *   u32 srvIndex = tex->GetShaderView();
	 * @endcode
	 */
	class GfxTexture : public GfxObject
	{
	public:
		const GfxTextureDesc& GetDesc() const { return m_desc; }

		u32 GetShaderView() const { return m_shaderView; }
		u32 GetStorageView() const { return m_storageView; }
		u32 GetRenderTargetView() const { return m_renderTargetView; }
		u32 GetDepthStencilView() const { return m_depthStencilView; }

	protected:
		GfxTextureDesc m_desc{};

		u32 m_shaderView = kInvalid32;
		u32 m_storageView = kInvalid32;
		u32 m_renderTargetView = kInvalid32;
		u32 m_depthStencilView = kInvalid32;
	};
}
