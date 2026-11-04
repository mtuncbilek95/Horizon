#pragma once

#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Engine
{
	enum class RenderResourceType : u8
	{
		Texture,
		Buffer
	};

	struct RenderResource
	{
		RenderResourceType type = RenderResourceType::Texture;

		RHI::TextureDesc texDesc;
		RHI::BufferDesc bufDesc;

		UniquePtr<RHI::Texture> texture;
		UniquePtr<RHI::TextureView> texView;
		UniquePtr<RHI::Buffer> buffer;
	};
}