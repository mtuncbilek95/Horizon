#pragma once

#include <Runtime/RHI/Texture/Texture.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon::RHI
{
	class D3D12Texture final : public Texture
	{
		friend class D3D12Device;
		friend class D3D12Swapchain;
	public:
		~D3D12Texture() final;

		void SetDebugName(const c8* pName) final;

		ID3D12Resource* Handle() const { return m_resource; }
		D3D12MA::Allocation* Allocation() const { return m_allocation; }

	private:
		ID3D12Resource* m_resource = nullptr;
		D3D12MA::Allocation* m_allocation = nullptr;
	};
}