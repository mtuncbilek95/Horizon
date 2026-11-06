#pragma once

#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Pipeline/Pipeline.h>

namespace Horizon::RHI
{
	class GraphicsPipeline : public Pipeline
	{
	public:
		PrimitiveTopology GetTopology() const { return m_topology; }
		const List<VertexBinding>& GetVertexBindings() const { return m_vertexBindings; }
		b8 IsMeshPipeline() const { return m_isMeshPipeline; }

	protected:
		PrimitiveTopology m_topology = PrimitiveTopology::TriangleList;
		List<VertexBinding> m_vertexBindings;
		b8 m_isMeshPipeline = false;
	};
}
