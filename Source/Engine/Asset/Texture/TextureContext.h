#pragma once

#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Asset
{
	class Texture;

	class TextureContext
	{
		friend class TextureStreamer;
	public:
		TextureContext() = default;
		~TextureContext() = default;

		TextureContext(const TextureContext&) = delete;
		TextureContext& operator=(const TextureContext&) = delete;

		RHI::TextureView* GetView() const { return m_view.Get(); }
		u32 GetViewIndex() const { return m_viewIndex; }

	private:
		Texture* m_pointedAsset = nullptr;

		UniquePtr<RHI::Texture> m_texture = nullptr;
		UniquePtr<RHI::TextureView> m_view = nullptr;
		u32 m_viewIndex = kInvalid32;
	};
}