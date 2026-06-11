#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_dx12.h>

#include <GLFW/glfw3.h>

namespace Horizon
{
	static GfxDescriptorHeap* gImGuiHeap = nullptr;

	void Gfx::InitGfxImGui(GfxDevice* pDevice, GfxQueue* pGraphicsQueue, GfxDescriptorHeap* pResourceHeap,
		void* pWindowHandle, GfxTextureFormat targetFormat, u32 framesInFlight)
	{
		gImGuiHeap = pResourceHeap;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForOther((GLFWwindow*)pWindowHandle, true);

		ImGui_ImplDX12_InitInfo initInfo = {};
		initInfo.Device = pDevice->pDevice;
		initInfo.CommandQueue = pGraphicsQueue->pQueue;
		initInfo.NumFramesInFlight = i32(framesInFlight);
		initInfo.RTVFormat = Helpers::ToDXGIFormat(targetFormat);
		initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
		initInfo.SrvDescriptorHeap = pResourceHeap->pHeap;
		initInfo.UserData = pResourceHeap;

		initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* pInfo,
			D3D12_CPU_DESCRIPTOR_HANDLE* pOutCpu, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGpu)
			{
				GfxDescriptorHeap* pHeap = (GfxDescriptorHeap*)pInfo->UserData;
				const u32 index = Helpers::AllocateDescriptor(pHeap);
				*pOutCpu = Helpers::CpuAt(pHeap, index);
				*pOutGpu = Helpers::GpuAt(pHeap, index);
			};

		initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* pInfo,
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
			{
				GfxDescriptorHeap* pHeap = (GfxDescriptorHeap*)pInfo->UserData;
				const u32 index = u32((cpuHandle.ptr - pHeap->cpuStart.ptr) / pHeap->descriptorSize);
				Helpers::FreeDescriptor(pHeap, index);
			};

		ImGui_ImplDX12_Init(&initInfo);
	}

	void Gfx::NewGfxImGuiFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Gfx::RenderGfxImGui(GfxCmdList* pCmd)
	{
		ImGui::Render();

		ID3D12DescriptorHeap* heaps[] = { gImGuiHeap->pHeap };
		pCmd->pList->SetDescriptorHeaps(1, heaps);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmd->pList);
	}

	void Gfx::ShutdownGfxImGui()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		gImGuiHeap = nullptr;
	}
}
