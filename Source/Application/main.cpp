#include <Runtime/Window/Window.h>

#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Shader/GfxShader.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/ShaderCompiler/ShaderCompiler.h>
#include <Runtime/Data/Geometry.h>

#include <filesystem>

using namespace Horizon;

static const std::filesystem::path gVertexShaderPath = "Shaders/triangle.vert";
static const std::filesystem::path gFragmentShaderPath = "Shaders/triangle.frag";

int main(int argc, char* argv[])
{
	auto window = Window(WindowDesc()
		.setWindowMode(WindowMode::Windowed)
		.setWindowName("Test")
		.setWindowSize({ 1920u, 1080u }));
	window.Show();

	auto instance = GfxInstance::Create(GfxInstanceDesc()
		.setAPIType(GfxType::Vulkan)
		.setAppName("Test")
		.setAppVersion({ 1, 0, 0 }));

	auto device = instance->CreateDevice(GfxDeviceDesc()
		.setComputeQueueCount(1)
		.setGraphicsQueueCount(1)
		.setTransferQueueCount(1));
	auto gQueue = device->CreateQueue(QueueType::Graphics);
	auto tQueue = device->CreateQueue(QueueType::Transfer);

	auto swapchain = device->CreateSwapchain(GfxSwapchainDesc()
		.setFormat(ImageFormat::R8G8B8A8_UNorm)
		.setGraphicsQueue(gQueue.get())
		.setImageCount(2)
		.setImageSize(window.GetSize())
		.setPresentMode(PresentMode::Mailbox)
		.setWindowHandler(window.GetHandle())
		.setWindowInstance(window.GetInstance())
		.setWindowAPI(window.GetNativeWindow()));

	auto cmdPool = device->CreateCommandPool(GfxCommandPoolDesc()
		.setQueue(gQueue.get())
		.setFlags(CommandPoolFlags::ResetCommandBuffer));
	auto cmdBuffer = device->CreateCommandBuffer(GfxCommandBufferDesc()
		.setPool(cmdPool.get())
		.setLevel(CommandLevel::Primary));

	auto fence = device->CreateSyncFence(GfxFenceDesc()
		.setSignal(true));
	auto imageAvailable = device->CreateSyncSemaphore(GfxSemaphoreDesc());
	auto renderFinished = device->CreateSyncSemaphore(GfxSemaphoreDesc());

	auto vertData = ShaderCompiler::ReadShaderData(gVertexShaderPath);
	auto fragData = ShaderCompiler::ReadShaderData(gFragmentShaderPath);
	auto vertSpirv = ShaderCompiler::GenerateSpirv(vertData.sourceText, vertData.entryPoint, vertData.stage);
	auto fragSpirv = ShaderCompiler::GenerateSpirv(fragData.sourceText, fragData.entryPoint, fragData.stage);

	auto vertShader = device->CreateShader(GfxShaderDesc()
		.setStage(ShaderStage::Vertex)
		.setByteCode(*vertSpirv));
	auto fragShader = device->CreateShader(GfxShaderDesc()
		.setStage(ShaderStage::Fragment)
		.setByteCode(*fragSpirv));

	auto windowSize = window.GetSize();

	auto pipeline = device->CreateGraphicsPipeline(GfxGraphicsPipelineDesc()
		.addShader(vertShader.get())
		.addShader(fragShader.get())
		.setInput(InputAssembler()
			.setTopology(PrimitiveTopology::TriangleList)
			.setPrimitiveRestart(false)
			.addBinding(InputBinding()
				.setInputRate(VertexInputRate::Vertex)
				.addAttribute(ImageFormat::R32G32B32_SFloat)
				.addAttribute(ImageFormat::R32G32B32_SFloat)
				.addAttribute(ImageFormat::R32G32B32_SFloat)
				.addAttribute(ImageFormat::R32G32B32_SFloat)
				.addAttribute(ImageFormat::R32G32B32A32_SFloat)
				.addAttribute(ImageFormat::R32G32_SFloat)))
		.setRasterizer(RasterizerState()
			.setPolygonMode(PolygonMode::Fill)
			.setCullMode(CullMode::Back)
			.setFaceOrientation(FaceOrientation::CCW)
			.setDepthBiasEnable(false))
		.setBlend(BlendState()
			.setLogicEnable(false)
			.addAttachment(BlendAttachment()
				.setBlendEnable(false)
				.setColorMask(ColorComponent::All)))
		.setDepthStencil(DepthStencilState()
			.setDepthTestEnable(false)
			.setDepthWriteEnable(false))
		.setViewport(Viewport()
			.setPosition({ 0.f, 0.f })
			.setSize({ static_cast<f32>(windowSize.x), static_cast<f32>(windowSize.y) })
			.setDepth({ 0.f, 1.f }))
		.setScissor(Scissor()
			.setOffset({ 0, 0 })
			.setExtent(windowSize))
		.setRenderPass(nullptr)
		.addDynamicState(DynamicState::Viewport)
		.addDynamicState(DynamicState::Scissor)
		.setDynamicRendering(DynamicRendering()
			.addColorAttachmentFormat(ImageFormat::R8G8B8A8_UNorm)));

	auto geometry = Geometry(3, 3);
	geometry.AddPosition({ 0.f, 0.5f, 0.f }, 0);
	geometry.AddPosition({ 0.5f, -0.5f, 0.f }, 1);
	geometry.AddPosition({ -0.5f, -0.5f, 0.f }, 2);
	geometry.AddIndex({ 0, 1, 2 });

	auto rawVertex = geometry.GenerateRawVertex(VertexRawDataFlags::All);
	auto rawIndex = geometry.GenerateRawIndex();

	auto vertexBuffer = device->CreateBuffer(GfxBufferDesc()
		.setSize(rawVertex.size())
		.setUsage(BufferUsage::Vertex | BufferUsage::TransferDst)
		.setMemoryUsage(MemoryUsage::AutoPreferDevice)
		.setAllocationFlags(MemoryAllocation::StrategyMinMemory));

	auto indexBuffer = device->CreateBuffer(GfxBufferDesc()
		.setSize(rawIndex.size())
		.setUsage(BufferUsage::Index | BufferUsage::TransferDst)
		.setMemoryUsage(MemoryUsage::AutoPreferDevice)
		.setAllocationFlags(MemoryAllocation::StrategyMinMemory));

	auto stagingVertex = device->CreateBuffer(GfxBufferDesc()
		.setSize(rawVertex.size())
		.setUsage(BufferUsage::TransferSrc)
		.setMemoryUsage(MemoryUsage::AutoPreferHost)
		.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite));
	stagingVertex->Update(rawVertex.data(), rawVertex.size());

	auto stagingIndex = device->CreateBuffer(GfxBufferDesc()
		.setSize(rawIndex.size())
		.setUsage(BufferUsage::TransferSrc)
		.setMemoryUsage(MemoryUsage::AutoPreferHost)
		.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite));
	stagingIndex->Update(rawIndex.data(), rawIndex.size());

	cmdBuffer->BeginRecord();
	cmdBuffer->CopyStageToBuffer(CopyBufferDesc()
		.setSrc(stagingVertex.get())
		.setDst(vertexBuffer.get())
		.setSize(static_cast<u32>(rawVertex.size())));
	cmdBuffer->CopyStageToBuffer(CopyBufferDesc()
		.setSrc(stagingIndex.get())
		.setDst(indexBuffer.get())
		.setSize(static_cast<u32>(rawIndex.size())));
	cmdBuffer->EndRecord();
	gQueue->Submit({ cmdBuffer.get() }, {}, {}, nullptr, PipelineStageFlags::Transfer);
	gQueue->WaitIdle();

	while (window.IsActive())
	{
		window.ProcessEvents();

		fence->WaitIdle();
		fence->Reset();

		auto imageIndex = swapchain->AcquireNextImage(imageAvailable.get(), nullptr);

		cmdBuffer->BeginRecord();

		// STOP: GfxCommandBuffer is missing BeginRendering() / EndRendering().
		// Dynamic rendering (VK_KHR_dynamic_rendering) needs these to bind the
		// swapchain color attachment per frame. Add them to GfxCommandBuffer first.

		cmdBuffer->EndRecord();

		gQueue->Submit(
			{ cmdBuffer.get() },
			{ imageAvailable.get() },
			{ renderFinished.get() },
			fence.get(),
			PipelineStageFlags::ColorAttachment);

		swapchain->Present({ renderFinished.get() });
	}

	device->WaitIdle();
}
