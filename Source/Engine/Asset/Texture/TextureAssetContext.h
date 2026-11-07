#pragma once

#include <Engine/Asset/Texture/TextureAsset.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Engine
{
	class TextureAssetContext
	{
	public:

	private:
		TextureAsset* m_pointedAsset = nullptr;

		UniquePtr<RHI::Texture> m_texture = nullptr;
		UniquePtr<RHI::TextureView> m_view = nullptr;
	};
}