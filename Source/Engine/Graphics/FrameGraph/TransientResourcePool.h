#pragma once

#include <Runtime/Graphics/RHI/Texture/GfxTexture.h>
#include <Runtime/Graphics/RHI/Object/GfxPointer.h>

#include <vector>

namespace Horizon
{
	class GfxDevice;

	class TransientResourcePool
	{
		struct Bucket
		{
			GfxTextureDesc desc{};
			std::vector<GfxPointer<GfxTexture>> owned;
			std::vector<GfxTexture*> free;
		};

	public:
		void Init(GfxDevice* device);

		GfxTexture* AcquireTexture(const GfxTextureDesc& desc);
		void ReleaseTexture(GfxTexture* texture);

	private:
		Bucket& BucketFor(const GfxTextureDesc& desc);

	private:
		GfxDevice* m_device = nullptr;
		std::vector<Bucket> m_buckets;
	};
}