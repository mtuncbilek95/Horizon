#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxDescriptorHeap* Gfx::CreateGfxDescriptorHeap(GfxDevice* pContext, const GfxDescriptorHeapDesc& desc)
	{
		assert((!desc.shaderVisible || desc.type == GfxDescriptorHeapType::Resource) && "Only CBV_SRV_UAV heaps can be shader-visible");

		GfxDescriptorHeap* pHeap = new GfxDescriptorHeap();

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = desc.type == GfxDescriptorHeapType::Resource ? D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV :
			desc.type == GfxDescriptorHeapType::Color ? D3D12_DESCRIPTOR_HEAP_TYPE_RTV : D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.NumDescriptors = desc.capacity;
		heapDesc.Flags = desc.shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		HRESULT bResult = pContext->pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pHeap->pHeap));
		CHECK_HR(bResult, "ID3D12DescriptorHeap - CreateDescriptorHeap");

		pHeap->descriptorSize = pContext->pDevice->GetDescriptorHandleIncrementSize(heapDesc.Type);
		pHeap->capacity = desc.capacity;
		pHeap->cpuStart = pHeap->pHeap->GetCPUDescriptorHandleForHeapStart();
		pHeap->gpuStart = desc.shaderVisible ? pHeap->pHeap->GetGPUDescriptorHandleForHeapStart()
			: D3D12_GPU_DESCRIPTOR_HANDLE{};

		pHeap->top = 0;
		pHeap->freeList.clear();

		return pHeap;
	}

	void Gfx::FreeDescriptorIndex(GfxDescriptorHeap* pHeap, u32 index)
	{
		Helpers::FreeDescriptor(pHeap, index);
	}

	void Gfx::DestroyGfxDescriptorHeap(GfxDescriptorHeap* heapHandl)
	{
		if (heapHandl->pHeap)
			heapHandl->pHeap->Release();

		delete heapHandl;
	}
}