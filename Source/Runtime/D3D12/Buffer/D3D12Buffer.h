#pragma once

#include <Runtime/RHI/Buffer/Buffer.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon::RHI
{
	class D3D12Buffer final : public Buffer
	{
		friend class D3D12Device;
	public:
		~D3D12Buffer() final;

		void SetDebugName(const c8* pName) final;

		ID3D12Resource* Handle() const { return m_resource; }
		D3D12MA::Allocation* Allocation() const { return m_allocation; }

	private:
		ID3D12Resource* m_resource = nullptr;
		D3D12MA::Allocation* m_allocation = nullptr;
	};
}