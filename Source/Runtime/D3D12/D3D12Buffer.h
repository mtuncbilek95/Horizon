#pragma once

#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/D3D12/D3D12Backend.h>

namespace D3D12MA { class Allocation; }

namespace Horizon
{
	class D3D12Device;

	class D3D12Buffer final : public GfxBuffer
	{
		friend class D3D12Device;
	public:
		~D3D12Buffer() final;

		ID3D12Resource* GetResource() const { return m_resource; }

	protected:
		ID3D12Resource* m_resource = nullptr;
		D3D12MA::Allocation* m_allocation = nullptr;
	};
}