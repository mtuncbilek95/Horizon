#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/D3D12/D3D12Backend.h>

namespace Horizon
{
	class D3D12Device;

	class D3D12Pipeline final : public GfxPipeline
	{
		friend class D3D12Device;
	public:
		~D3D12Pipeline() final { if (m_pipeline) m_pipeline->Release(); }

		ID3D12PipelineState* GetPipeline() const { return m_pipeline; }
		D3D_PRIMITIVE_TOPOLOGY GetTopology() const { return m_topology; }
		b8 UsesMeshShading() const { return m_usesMeshShading; }

	private:
		ID3D12PipelineState* m_pipeline = nullptr;
		D3D_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		b8 m_usesMeshShading = false;
	};
}