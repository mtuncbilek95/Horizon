#include "D3D12Sampler.h"

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/RHI/Device/GfxDevice.h>

namespace Horizon
{
	D3D12Sampler::~D3D12Sampler()
	{
		m_ownerDevice->GetDescriptorHeap(GfxDescriptorHeapType::Sampler)->Free(m_heapIndex);
	}
}