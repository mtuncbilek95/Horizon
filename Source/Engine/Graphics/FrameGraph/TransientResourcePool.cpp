#include "TransientResourcePool.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

namespace Horizon
{
	static b8 SameDesc(const GfxTextureDesc& a, const GfxTextureDesc& b)
	{
		return a.width == b.width && a.height == b.height && a.depth == b.depth
			&& a.mipLevels == b.mipLevels && a.format == b.format
			&& a.usage == b.usage && a.type == b.type && a.clearColor == b.clearColor;
	}

	void TransientResourcePool::Init(GfxDevice* device)
	{
		m_device = device;
	}

	TransientResourcePool::Bucket& TransientResourcePool::BucketFor(const GfxTextureDesc& desc)
	{
		for (Bucket& bucket : m_buckets)
			if (SameDesc(bucket.desc, desc))
				return bucket;
		Bucket bucket;
		bucket.desc = desc;
		m_buckets.push_back(std::move(bucket));
		return m_buckets.back();
	}

	GfxTexture* TransientResourcePool::AcquireTexture(const GfxTextureDesc& desc)
	{
		Bucket& bucket = BucketFor(desc);
		if (!bucket.free.empty())
		{
			GfxTexture* texture = bucket.free.back();
			bucket.free.pop_back();
			return texture;
		}
		GfxPointer<GfxTexture> texture = m_device->CreateTexture(desc);
		GfxTexture* raw = texture.GetRawPtr();
		bucket.owned.push_back(std::move(texture));
		return raw;
	}

	void TransientResourcePool::ReleaseTexture(GfxTexture* texture)
	{
		BucketFor(texture->GetDesc()).free.push_back(texture);
	}
}