#pragma once

#include <Engine/Asset/Texture/Texture.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Asset
{
	class TextureContext
	{
	public:

	private:
		Texture* m_pointedAsset = nullptr;

		UniquePtr<RHI::Texture> m_texture = nullptr;
		UniquePtr<RHI::TextureView> m_view = nullptr;
	};
}