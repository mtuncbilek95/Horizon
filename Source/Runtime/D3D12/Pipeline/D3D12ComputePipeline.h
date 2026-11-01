#pragma once

#include <Runtime/RHI/Pipeline/ComputePipeline.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12ComputePipeline final : public ComputePipeline
	{
		friend class D3D12Device;
	public:
		~D3D12ComputePipeline() final;

		void SetDebugName(const c8* pName) final;

		ID3D12PipelineState* Handle() const { return m_pipelineState; }

	private:
		ID3D12PipelineState* m_pipelineState = nullptr;
	};
}
