#pragma once

#include <Engine/Presentation/PresentationObjects.h>

namespace Horizon
{
	class FrameGraphCache;

	template<typename Tag>
	struct ResourceHandle
	{
		u32 idx = 0;
		u32 version = 0;

		b8 isValid() const { return idx != std::numeric_limits<u32>(); }
		b8 operator==(const ResourceHandle& other) { return idx == other.idx; }
		b8 operator!=(const ResourceHandle& other) { return idx != other.idx; }
	};

	struct RenderTargetTag;
	using RenderTargetHandle = ResourceHandle<RenderTargetTag>;

	class FrameGraph
	{
	public:
		
		void ResolveCompositePass(const CompositePresentObject& passObj);
		void SetGraphCache(std::unique_ptr<FrameGraphCache> cache);

	private:
		std::unique_ptr<FrameGraphCache> m_cache;
		CompositePresentObject m_composite;
	};
}