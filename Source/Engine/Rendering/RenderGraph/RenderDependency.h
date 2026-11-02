#pragma once

#include <Engine/Rendering/RenderGraph/RenderAccess.h>
#include <Runtime/Containers/Vector/List.h>

namespace Horizon::Engine
{
	class RenderDependency final
	{
		friend class RenderGraph;

	public:
		void Reads(RenderResource resource, RHI::ResourceState state)
		{
			m_reads.PushBack({ resource, state });
		}

		RenderResource Writes(RenderResource resource, RHI::ResourceState state)
		{
			m_writes.PushBack({ resource, state });
			return { resource.index, resource.version + 1 };
		}

	private:
		List<RenderAccess> m_reads;
		List<RenderAccess> m_writes;
	};
}