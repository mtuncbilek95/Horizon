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
#include <Runtime/Camera/Camera.h>

#include <filesystem>
#include <print>
#include <chrono>

using namespace Horizon;

static Geometry CreateDonut(f32 majorRadius, f32 minorRadius, u32 majorSegments, u32 minorSegments)
{
    Geometry geo;

    for (u32 i = 0; i < majorSegments; i++)
    {
        f32 u = static_cast<f32>(i) / static_cast<f32>(majorSegments) * 2.f * Math::Pi;
        for (u32 j = 0; j < minorSegments; j++)
        {
            f32 v = static_cast<f32>(j) / static_cast<f32>(minorSegments) * 2.f * Math::Pi;

            f32 x = (majorRadius + minorRadius * glm::cos(v)) * glm::cos(u);
            f32 y = (majorRadius + minorRadius * glm::cos(v)) * glm::sin(u);
            f32 z = minorRadius * glm::sin(v);

            f32 nx = glm::cos(v) * glm::cos(u);
            f32 ny = glm::cos(v) * glm::sin(u);
            f32 nz = glm::sin(v);

            u32 vIdx = i * minorSegments + j;
            geo.AddPosition({ x, y, z }, vIdx);
            geo.AddNormal({ nx, ny, nz }, vIdx);
            geo.AddColor({ nx * 0.5f + 0.5f, ny * 0.5f + 0.5f, nz * 0.5f + 0.5f, 1.f }, vIdx);
        }
    }

    std::vector<u32> indices;
    indices.reserve(majorSegments * minorSegments * 6);
    for (u32 i = 0; i < majorSegments; i++)
    {
        u32 ni = (i + 1) % majorSegments;
        for (u32 j = 0; j < minorSegments; j++)
        {
            u32 nj = (j + 1) % minorSegments;

            u32 a = i  * minorSegments + j;
            u32 b = ni * minorSegments + j;
            u32 c = ni * minorSegments + nj;
            u32 d = i  * minorSegments + nj;

            indices.insert(indices.end(), { a, b, c, a, c, d });
        }
    }
    geo.AddIndex(indices);

    return geo;
}

static const std::string gVertexShader = R"(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pc;

void main()
{
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    fragColor = inColor;
}
)";

static const std::string gFragmentShader = R"(
#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;
void main()
{
    outColor = fragColor;
}
)";

int main(int argc, char* argv[])
{
	auto window = Window(WindowDesc()
		.setWindowMode(WindowMode::Windowed)
		.setWindowName("Test")
		.setWindowSize({ 1920u, 1080u }));
	window.Show();

	auto camera = Camera(window.GetInputDispatcher());

	auto instance = GfxInstance::Create(GfxInstanceDesc()
		.setAPIType(GfxType::Vulkan)
		.setAppName("Test")
		.setAppVersion({ 1, 0, 0 }));
	
	auto device = instance->CreateDevice(GfxDeviceDesc()
		.setComputeQueueCount(1)
		.setGraphicsQueueCount(1)
		.setTransferQueueCount(1));

	auto gQueue = device->CreateQueue(QueueType::Graphics);
	
	auto swapchain = device->CreateSwapchain(GfxSwapchainDesc()
		.setFormat(ImageFormat::R8G8B8A8_UNorm)
		.setGraphicsQueue(gQueue.get())
		.setImageCount(3)
		.setImageSize(window.GetSize())
		.setPresentMode(PresentMode::Mailbox)
		.setWindowAPI(window.GetNativeWindow())
		.setWindowHandler(window.GetHandle())
		.setWindowInstance(window.GetInstance()));

	// --- Depth buffer ---
	auto depthImage = device->CreateImage(GfxImageDesc()
		.setSize({ window.GetSize().x, window.GetSize().y, 1})
		.setFormat(ImageFormat::D32_SFloat)
		.setUsage(ImageUsage::DepthStencil)
		.setMemoryUsage(MemoryUsage::AutoPreferDevice)
		.setMemoryFlags(MemoryAllocation::DedicatedMemory));

	auto depthView = device->CreateView(GfxImageViewDesc()
		.setImage(depthImage.get())
		.setViewType(ImageViewType::View2D)
		.setAspect(ImageAspect::Depth));

	auto geo = CreateDonut(0.5f, 0.2f, 64, 32);

	auto rawVerts   = geo.GenerateRawVertex(VertexRawDataFlags::Position | VertexRawDataFlags::Color);
	auto rawIndices = geo.GenerateRawIndex();

	auto vBuffer = device->CreateBuffer(GfxBufferDesc()
		.setSize(rawVerts.size())
		.setUsage(BufferUsage::Vertex)
		.setMemoryUsage(MemoryUsage::AutoPreferHost)
		.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::HostAccessRandom));
	vBuffer->Update(rawVerts.data(), rawVerts.size());

	auto iBuffer = device->CreateBuffer(GfxBufferDesc()
		.setSize(rawIndices.size())
		.setUsage(BufferUsage::Index)
		.setMemoryUsage(MemoryUsage::AutoPreferHost)
		.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::HostAccessRandom));
	iBuffer->Update(rawIndices.data(), rawIndices.size());

	// --- Shaders ---
	auto vertSpirv = ShaderCompiler::GenerateSpirv(gVertexShader, "main", ShaderStage::Vertex);
	auto fragSpirv = ShaderCompiler::GenerateSpirv(gFragmentShader, "main", ShaderStage::Fragment);

	auto vertShader = device->CreateShader(GfxShaderDesc()
		.setStage(ShaderStage::Vertex)
		.setByteCode(*vertSpirv));
	auto fragShader = device->CreateShader(GfxShaderDesc()
		.setStage(ShaderStage::Fragment)
		.setByteCode(*fragSpirv));

	auto size = swapchain->ImageSize();

	auto pipeline = device->CreateGraphicsPipeline(GfxGraphicsPipelineDesc()
		.addShader(vertShader.get())
		.addShader(fragShader.get())
		.setInput(InputAssembler()
			.setTopology(PrimitiveTopology::TriangleList)
			.addBinding(InputBinding()
				.setInputRate(VertexInputRate::Vertex)
				.addAttribute(ImageFormat::R32G32B32_SFloat)
				.addAttribute(ImageFormat::R32G32B32A32_SFloat)
			))
		.setRasterizer(RasterizerState()
			.setPolygonMode(PolygonMode::Fill)
			.setCullMode(CullMode::Back)
			.setFaceOrientation(FaceOrientation::CCW))
		.setBlend(BlendState()
			.addAttachment(BlendAttachment()
				.setBlendEnable(false)
				.setColorMask(ColorComponent::All)))
		.setDepthStencil(DepthStencilState()
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setDepthOp(CompareOp::Less))
		.setViewport(Viewport()
			.setPosition({ 0.f, 0.f })
			.setSize({ static_cast<f32>(size.x), static_cast<f32>(size.y) })
			.setDepth({ 0.f, 1.f }))
		.setScissor(Scissor()
			.setOffset({ 0, 0 })
			.setExtent(size))
		.setPushConstants(PushConstants()
			.addRange(PushConstantRange()
				.setStage(ShaderStage::Vertex)
				.setOffset(0)
				.setSizeOfData(sizeof(Math::Mat4f))))
		.setDynamicRendering(DynamicRendering()
			.addColorAttachmentFormat(swapchain->ImageFormat())
			.setDepthAttachmentFormat(ImageFormat::D32_SFloat)));

	auto cPool = device->CreateCommandPool(GfxCommandPoolDesc()
		.setQueue(gQueue.get())
		.setFlags(CommandPoolFlags::ResetCommandBuffer));

	std::vector<std::shared_ptr<GfxCommandBuffer>> cBuffers;
	for (size_t i = 0; i < swapchain->ImageCount(); i++)
		cBuffers.push_back(device->CreateCommandBuffer(GfxCommandBufferDesc()
			.setLevel(CommandLevel::Primary)
			.setPool(cPool.get())));

	std::vector<std::shared_ptr<GfxFence>> fences;
	for (size_t i = 0; i < swapchain->ImageCount(); i++)
		fences.push_back(device->CreateSyncFence(GfxFenceDesc().setSignal(true)));

	std::vector<std::shared_ptr<GfxSemaphore>> imageAvailableSemaphores;
	std::vector<std::shared_ptr<GfxSemaphore>> renderFinishedSemaphores;
	for (size_t i = 0; i < swapchain->ImageCount(); i++)
	{
		imageAvailableSemaphores.push_back(device->CreateSyncSemaphore(GfxSemaphoreDesc()));
		renderFinishedSemaphores.push_back(device->CreateSyncSemaphore(GfxSemaphoreDesc()));
	}

	u32 frameIndex = 0;
	auto lastTime = std::chrono::steady_clock::now();

	while (window.IsActive())
	{
		auto now = std::chrono::steady_clock::now();
		f32 deltaTime = std::chrono::duration<f32>(now - lastTime).count();
		lastTime = now;

		window.ProcessEvents();
		camera.Update(deltaTime);

		fences[frameIndex]->WaitIdle();
		fences[frameIndex]->Reset();

		u32 imageIndex = swapchain->AcquireNextImage(imageAvailableSemaphores[frameIndex].get(), nullptr);

		auto* swapImage = swapchain->Image(imageIndex);

		cBuffers[frameIndex]->BeginRecord();

		cBuffers[frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(swapImage)
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::ColorAttachmentOptimal)
			.setAspect(ImageAspect::Color));

		cBuffers[frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(depthImage.get())
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::DepthStencilAttachmentOptimal)
			.setAspect(ImageAspect::Depth));

		RenderingAttachmentInfo depthAttachment = RenderingAttachmentInfo()
			.setImageView(depthView.get())
			.setImageLayout(ImageLayout::DepthStencilAttachmentOptimal)
			.setLoadOp(AttachmentLoad::Clear)
			.setStoreOp(AttachmentStore::DontCare)
			.setClearValue(ClearValue().setDepth(1.f));

		cBuffers[frameIndex]->BeginRendering(RenderingInfo()
			.setRenderAreaExtent(swapchain->ImageSize())
			.addColorAttachment(RenderingAttachmentInfo()
				.setImageView(swapchain->ImageView(imageIndex))
				.setImageLayout(ImageLayout::ColorAttachmentOptimal)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
				.setClearValue(ClearValue().setColor({ 1.f, 1.f, 0.f, 1.f })))
			.setDepthAttachment(&depthAttachment));

		auto mvp = camera.GetProjectionMatrix(60.f, static_cast<f32>(size.x) / static_cast<f32>(size.y), 0.01f, 1000.f)
		         * camera.GetViewMatrix();

		cBuffers[frameIndex]->BindPipeline(pipeline.get());
		cBuffers[frameIndex]->BindPushConstants(ShaderStage::Vertex, 0, sizeof(Math::Mat4f), &mvp);
		cBuffers[frameIndex]->BindVertices({ vBuffer.get() });
		cBuffers[frameIndex]->BindIndex(iBuffer.get(), 0);
		cBuffers[frameIndex]->DrawIndexed(static_cast<u32>(geo.GetIndexCount()), 0, 0, 0, 1);

		cBuffers[frameIndex]->EndRendering();

		cBuffers[frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(swapImage)
			.setOldLayout(ImageLayout::ColorAttachmentOptimal)
			.setNewLayout(ImageLayout::PresentSrcKHR)
			.setAspect(ImageAspect::Color));

		cBuffers[frameIndex]->EndRecord();

		gQueue->Submit(
			{ cBuffers[frameIndex].get() },
			{ imageAvailableSemaphores[frameIndex].get() },
			{ renderFinishedSemaphores[frameIndex].get() },
			fences[frameIndex].get(),
			PipelineStageFlags::ColorAttachment);

		swapchain->Present({ renderFinishedSemaphores[frameIndex].get() });

		frameIndex = (frameIndex + 1) % swapchain->ImageCount();
	}

	device->WaitIdle();
}