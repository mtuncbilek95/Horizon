#include <Engine/Graphics/RHI/GfxImGui.h>
#include <Engine/Graphics/DX12/DX12Backend.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>

#include <GLFW/glfw3.h>

namespace Horizon::GfxImGui
{
	using namespace DX12;

	static u32 s_fontSrvSlot = ~0u;

	void Init(void* glfwWindow)
	{
		Context& ctx = GfxContext();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOther((GLFWwindow*)glfwWindow, true);

		s_fontSrvSlot = BindlessAlloc();

		ImGui_ImplDX12_InitInfo init = {};
		init.Device = ctx.device;
		init.CommandQueue = ctx.queues[u32(GfxQueueType::Graphics)];
		init.NumFramesInFlight = (int)ctx.framesInFlight;
		init.RTVFormat = ctx.swapchainFormat;
		init.DSVFormat = DXGI_FORMAT_UNKNOWN;
		init.SrvDescriptorHeap = ctx.bindlessHeap;

		init.LegacySingleSrvCpuDescriptor = BindlessCpu(s_fontSrvSlot);
		init.LegacySingleSrvGpuDescriptor = BindlessGpu(s_fontSrvSlot);

		ImGui_ImplDX12_Init(&init);
	}

	void BeginFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Render(GfxCommandList* cmd)
	{
		Context& ctx = GfxContext();

		ImGui::Render();

		ID3D12DescriptorHeap* heaps[] = { ctx.bindlessHeap };
		cmd->list->SetDescriptorHeaps(1, heaps);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd->list);
	}

	void Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		if (s_fontSrvSlot != ~0u)
			BindlessFree(s_fontSrvSlot);
	}
}
