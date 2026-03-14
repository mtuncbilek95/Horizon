#include "EditorRenderer.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorPool.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/RenderPass/GfxRenderPass.h>

#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/Window/WindowSystem.h>

#include <volk/volk.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

namespace Horizon
{
	EditorRenderer::EditorRenderer() : m_editorContext(nullptr)
	{
	}

	SystemReport EditorRenderer::Initialize(ISystem* ownerSystem)
	{
		IMGUI_CHECKVERSION();

		m_editorContext = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		auto& windowSystem = RequestSystem<WindowSystem>();
		auto& graphicsSystem = static_cast<GraphicsSystem&>(*ownerSystem);

		if (!ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)windowSystem.GetAPIWindow(), true))
			return SystemReport(GetObjectType(), "Failed to initialize ImGui GLFW backend");

		ImGui_ImplVulkan_LoadFunctions(
			VK_API_VERSION_1_3,
			[](const char* functionName, void* userData) -> PFN_vkVoidFunction
			{
				return vkGetInstanceProcAddr(static_cast<VkInstance>(userData), functionName);
			},
			VkInstance(graphicsSystem.Instance()->Instance())
		);

		m_imguiDescriptorPool = graphicsSystem.Device()->CreateDescriptorPool(
			GfxDescriptorPoolDesc()
			.setFlags(DescriptorPoolFlags::FreeSets)
			.setMaxSets(1000)
			.addPoolSize({ DescriptorType::Sampler, 1000 })
			.addPoolSize({ DescriptorType::CombinedSamplerImage, 1000 })
			.addPoolSize({ DescriptorType::SampledImage, 1000 })
			.addPoolSize({ DescriptorType::StorageImage, 1000 })
			.addPoolSize({ DescriptorType::UniformTexelBuffer, 1000 })
			.addPoolSize({ DescriptorType::StorageTexelBuffer, 1000 })
			.addPoolSize({ DescriptorType::Uniform, 1000 })
			.addPoolSize({ DescriptorType::Storage, 1000 })
			.addPoolSize({ DescriptorType::DynamicUniform, 1000 })
			.addPoolSize({ DescriptorType::InputAttachment, 1000 })
		);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = VkInstance(graphicsSystem.Instance()->Instance());
		initInfo.PhysicalDevice = VkPhysicalDevice(graphicsSystem.Instance()->Adapter());
		initInfo.Device = VkDevice(graphicsSystem.Device()->Device());
		initInfo.QueueFamily = graphicsSystem.GraphicsQueue()->FamilyIndex();
		initInfo.Queue = VkQueue(graphicsSystem.GraphicsQueue()->Queue());
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = VkDescriptorPool(m_imguiDescriptorPool->DescPool());
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineInfoMain.SwapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		initInfo.PipelineInfoMain.RenderPass = VkRenderPass(graphicsSystem.PresentationPass()->Pass());
		initInfo.PipelineInfoMain.Subpass = 0;
		initInfo.ImageCount = 2;
		initInfo.MinImageCount = 2;
		initInfo.ApiVersion = VK_API_VERSION_1_3;

		if(!ImGui_ImplVulkan_Init(&initInfo))
			return SystemReport(GetObjectType(), "Failed to initialize ImGui Vulkan backend");

		return SystemReport();
	}

	void EditorRenderer::Render(GfxCommandBuffer* pDrawBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VkCommandBuffer(pDrawBuffer->Buffer()));
	}

	void EditorRenderer::Finalize()
	{
		RequestSystem<GraphicsSystem>().Device()->WaitIdle();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext((ImGuiContext*)m_editorContext);
	}
}