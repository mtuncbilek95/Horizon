#pragma once

#include <Runtime/RHI/Pipeline/PipelineLayout.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12PipelineLayout final : public PipelineLayout
	{
		friend class D3D12Device;
	public:
		static constexpr u32 kPushConstantSpace = 999;

		~D3D12PipelineLayout() final;

		void SetDebugName(const c8* pName) final;

		ID3D12RootSignature* Handle() const { return m_rootSignature; }
		u32 TableRootIndex(u32 tableIndex) const { return m_tableRootOffset + tableIndex; }

	private:
		ID3D12RootSignature* m_rootSignature = nullptr;
		u32 m_tableRootOffset = 0;
	};
}