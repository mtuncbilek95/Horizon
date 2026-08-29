#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Descriptor/GfxDescriptorSlot.h>
#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Texture/GfxTextureDesc.h>

namespace Horizon::RHI
{
	class GfxTexture : public GfxObject
	{
	public:
		const GfxTextureDesc& GetDesc() const { return m_desc; }

		u32 GetShaderView() const { return m_shaderView.index; }
		u32 GetStorageView() const { return m_storageView.index; }
		u32 GetRenderTargetView() const { return m_renderTargetView.index; }
		u32 GetDepthStencilView() const { return m_depthStencilView.index; }

	protected:
		void ReleaseViews();

		GfxTextureDesc m_desc{};

		GfxDescriptorSlot m_shaderView;
		GfxDescriptorSlot m_storageView;
		GfxDescriptorSlot m_renderTargetView;
		GfxDescriptorSlot m_depthStencilView;
	};
}
