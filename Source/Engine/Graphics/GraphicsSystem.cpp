#include "GraphicsSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Framebuffer/GfxFramebuffer.h>
#include <Runtime/Graphics/RHI/RenderPass/GfxRenderPass.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/Renderer/EditorRenderer.h>

#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>

#include <Runtime/ShaderCompiler/ShaderCompiler.h>

namespace Horizon
{
	struct Vertex {
		Math::Vec4f position;
		Math::Vec4f normal;
		Math::Vec4f tangent;
		Math::Vec2f uv;
		Math::Vec2f padding; // Padding to ensure 16-byte alignment
	};

	struct MeshPushConstants
	{
		alignas(16) Math::Mat4f model;
		alignas(16) Math::Mat4f viewProj;
		alignas(4) u32 triangleCount;
	};

	std::shared_ptr<GfxBuffer> m_cubeVertexBuffer;
	std::shared_ptr<GfxBuffer> m_cubeIndexBuffer;
	std::shared_ptr<GfxDescriptorLayout> m_meshDescriptorLayout;
	std::shared_ptr<GfxDescriptorBuffer> m_meshDescriptorBuffer;
	std::shared_ptr<GfxPipeline> m_meshPipeline;

	std::shared_ptr<GfxShader> m_meshShader;
	std::shared_ptr<GfxShader> m_fragShader;

	std::vector<Vertex> cubeVertices = {
		// Face -Z - Normal: {0, 0, -1}
		{{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},

		// Face +Z  - Normal: {0, 0, 1}
		{{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 0.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 0.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 0.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 0.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

		// Face -X - Normal: {-1, 0, 0}
		{{-0.5f, -0.5f, -0.5f, 1.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f, 1.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f, 1.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f, 1.0f}, {-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

		// Face +X  - Normal: {1, 0, 0}
		{{ 0.5f, -0.5f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},

		// Face -Y  - Normal: {0, -1, 0}
		{{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

		// Face +Y - Normal: {0, 1, 0}
		{{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f,  1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f,  1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.0f,  1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.0f,  1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
	};

	std::vector<uint32_t> cubeIndices = {
	0,  1,  2,  2,  3,  0,
	4,  5,  6,  6,  7,  4,
	8,  9,  10, 10, 11, 8,
	12, 13, 14, 14, 15, 12,
	16, 17, 18, 18, 19, 16,
	20, 21, 22, 22, 23, 20
	};

	GraphicsSystem::GraphicsSystem() : m_currentFrame(0), m_currentImageIndex(0)
	{
	}

	SystemReport GraphicsSystem::OnInitialize()
	{
		auto& windowSystem = RequestSystem<WindowSystem>();

		m_instance = GfxInstance::Create(
			GfxInstanceDesc()
			.setAppName("Horizon")
			.setAppVersion({ 1, 0, 0 })
			.setAPIType(GfxType::Vulkan)
		);
		m_device = m_instance->CreateDevice(
			GfxDeviceDesc()
			.setGraphicsQueueCount(1)
			.setComputeQueueCount(1)
			.setTransferQueueCount(0)
		);
		m_graphicsQueue = m_device->CreateQueue(QueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(QueueType::Compute);

		m_swapchain = m_device->CreateSwapchain(
			GfxSwapchainDesc()
			.setImageSize(windowSystem.GetWindowSize())
			.setImageCount(2)
			.setFormat(ImageFormat::R8G8B8A8_UNorm)
			.setPresentMode(PresentMode::Fifo)
			.setGraphicsQueue(m_graphicsQueue.get())
			.setWindowHandler(windowSystem.GetWindowHandle())
		);

		m_graphicsCmdPool = m_device->CreateCommandPool(
			GfxCommandPoolDesc()
			.setQueue(m_graphicsQueue.get())
			.setFlags(CommandPoolFlags::ResetCommandBuffer)
		);

		for (usize it = 0; it < m_swapchain->ImageCount(); it++)
		{
			m_graphicsCmds.push_back(m_graphicsCmdPool->CreateBuffer(CommandLevel::Primary));
			m_frameFences.push_back(m_device->CreateSyncFence(GfxFenceDesc().setSignal(false)));
			m_sceneFinishedSems.push_back(m_device->CreateSyncSemaphore(GfxSemaphoreDesc()));
		}

		m_presentationPass = m_device->CreateRenderPass(
			GfxRenderPassDesc()
			.addAttachment(
				AttachmentDesc()
				.setFormat(m_swapchain->ImageFormat())
				.setType(AttachmentType::Color)
				.setInitialLayout(ImageLayout::Undefined)
				.setFinalLayout(ImageLayout::PresentSrcKHR)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
			)
			.addSubpass(
				SubpassDesc()
				.addColorAttachment(0)
				.setBindPoint(PipelineType::Graphics)
			)
		);

		const Math::Vec2u& windowSize = m_swapchain->ImageSize();
		GfxFramebufferDesc fbDesc;
		fbDesc.setPassRef(m_presentationPass.get())
			.setFramebufferSize({ windowSize.x, windowSize.y, 1 });

		for (usize i = 0; i < m_swapchain->ImageCount(); i++)
			fbDesc.addFramebufferViews({ m_swapchain->ImageView(i) });
		m_presentationFramebuffer = m_device->CreateFramebuffer(fbDesc);

		// Initialize the editor renderer.
		m_editor = std::make_shared<EditorRenderer>();
		m_editor->SetEngine(GetEngine());
		if (!m_editor->Initialize(this))
			return SystemReport(GetObjectType(), "Failed to initialize editor renderer.");

		m_cubeVertexBuffer = m_device->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(cubeVertices.size() * sizeof(Vertex))
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_cubeVertexBuffer->Update(cubeVertices.data(), cubeVertices.size() * sizeof(Vertex));

		m_cubeIndexBuffer = m_device->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(cubeIndices.size() * sizeof(uint32_t))
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_cubeIndexBuffer->Update(cubeIndices.data(), cubeIndices.size() * sizeof(uint32_t));

		m_meshDescriptorLayout = m_device->CreateDescriptorLayout(
			GfxDescriptorLayoutDesc()
			.addBinding(DescriptorBinding()
				.setBinding(0)
				.setType(DescriptorType::Storage)
				.setStage(ShaderStage::Mesh)
				.setCount(1))
			.addBinding(DescriptorBinding()
				.setBinding(1)
				.setType(DescriptorType::Storage)
				.setStage(ShaderStage::Mesh)
				.setCount(1))
			.setFlags(DescriptorLayoutFlags::Buffer)
		);

		m_meshDescriptorBuffer = m_device->CreateDescriptorBuffer(
			GfxDescriptorBufferDesc()
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setMemoryUsage(MemoryUsage::AutoPreferDevice)
			.setSize(m_meshDescriptorLayout->SizeInBytes())
			.setType(DescriptorBufferType::Resource)
			.setUsage(DescriptorType::Storage)
		);
		m_meshDescriptorBuffer->RequestPayload(m_cubeVertexBuffer.get(), m_meshDescriptorLayout->BindingOffset(0));
		m_meshDescriptorBuffer->RequestPayload(m_cubeIndexBuffer.get(), m_meshDescriptorLayout->BindingOffset(1));

		//m_meshDescriptorBuffer->WriteDescriptor(m_meshDescriptorLayout->BindingOffset(0), m_cubeVertexBuffer->MappedData(), m_cubeVertexBuffer->MappedSize());
		//m_meshDescriptorBuffer->WriteDescriptor(m_meshDescriptorLayout->BindingOffset(1), m_cubeIndexBuffer->MappedData(), m_cubeIndexBuffer->MappedSize());

		std::string meshCode = DataReader::ReadToTextFile("../shaders/Meshlet.mesh");
		std::string fragCode = DataReader::ReadToTextFile("../shaders/Meshlet.frag");

		ReadArray<u32> meshSpirv = ShaderCompiler::GenerateSpirv(meshCode, "main", ShaderStage::Mesh);
		ReadArray<u32> fragSpirv = ShaderCompiler::GenerateSpirv(fragCode, "main", ShaderStage::Fragment);

		m_meshShader = m_device->CreateShader(
			GfxShaderDesc()
			.setStage(ShaderStage::Mesh)
			.setByteCode(*meshSpirv)
		);
		m_fragShader = m_device->CreateShader(
			GfxShaderDesc()
			.setStage(ShaderStage::Fragment)
			.setByteCode(*fragSpirv)
		);

		m_meshPipeline = m_device->CreateGraphicsPipeline(
			GfxGraphicsPipelineDesc()
			.addLayout(m_meshDescriptorLayout.get())
			.addShader(m_meshShader.get())
			.addShader(m_fragShader.get())
			.setRasterizer(
				RasterizerState()
				.setCullMode(CullMode::Back)
				.setFaceOrientation(FaceOrientation::CCW)
				.setPolygonMode(PolygonMode::Fill)
				.setDepthBiasEnable(false))
			.setPushConstants(
				PushConstants()
				.addRange(
					PushConstantRange()
					.setSizeOfData(sizeof(MeshPushConstants))
					.setStage(ShaderStage::Mesh)))
			.setInput(
				InputAssembler()
				.setTopology(PrimitiveTopology::TriangleList))
			.setBlend(
				BlendState()
				.setLogicEnable(false)
				.addAttachment(
					BlendAttachment()
					.setBlendEnable(false)
					.setColorMask(ColorComponent::All)))
			.setDepthStencil(
				DepthStencilState()
				.setDepthTestEnable(false)
				.setDepthWriteEnable(false)
				.setDepthOp(CompareOp::Less))
			.setViewport(Viewport().setSize({ 1920.f, 1080.f }))
			.setScissor(Scissor().setExtent({ 1920, 1080 }))
			.setRenderPass(m_presentationPass.get())
			.setFlags(PipelineFlags::DescriptorBuffer));

		return SystemReport();
	}

	void GraphicsSystem::OnSync()
	{
		m_currentImageIndex = m_swapchain->AcquireNextImage(nullptr, m_frameFences[m_currentFrame].get());
		m_frameFences[m_currentFrame]->WaitIdle();
		m_frameFences[m_currentFrame]->Reset();

		m_graphicsCmds[m_currentFrame]->BeginRecord(CommandBufferUsage::OneTimeSubmit);
		m_graphicsCmds[m_currentFrame]->BeginRenderPass(BeginRenderDesc()
			.setRenderPass(m_presentationPass.get())
			.setFramebuffer(m_presentationFramebuffer.get())
			.setFrameIndex(m_currentImageIndex)
			.setRenderArea(m_swapchain->ImageSize())
			.setColor({ .1f, .2f, .3f, 1.f })
			.setClearColor(true)
		);

		static glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, 0.006f, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 0.008f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 0.003f, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 30.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)
		);
		glm::mat4 proj = glm::perspective(glm::radians(45.0f),
			1920.f / 1080.f,
			0.1f, 100.0f);
		proj[1][1] *= -1;
		glm::mat4 viewProj = proj * view;

		auto pushData = MeshPushConstants{
			.model = model,
			.viewProj = viewProj,
			.triangleCount = 12
		};

		m_graphicsCmds[m_currentFrame]->BindPipeline(m_meshPipeline.get());
		m_graphicsCmds[m_currentFrame]->BindDescriptorBuffer(m_meshPipeline.get(), ShaderStage::Mesh, 0, m_meshDescriptorBuffer.get());
		m_graphicsCmds[m_currentFrame]->BindPushConstants(m_meshPipeline.get(), ShaderStage::Mesh, 0, sizeof(MeshPushConstants), &pushData);
		m_graphicsCmds[m_currentFrame]->DrawMeshTask(10000, 1, 1);

		m_editor->Render(m_graphicsCmds[m_currentFrame].get());

		m_graphicsCmds[m_currentFrame]->EndRenderPass();
		m_graphicsCmds[m_currentFrame]->EndRecord();

		auto& presentSys = RequestSystem<PresentationSystem>();
		presentSys.RequestPresent(PresentRequest()
			.setCmdBuffer(m_graphicsCmds[m_currentFrame].get())
			.setImageIndex(m_currentImageIndex)
		);
	}

	void GraphicsSystem::OnFinalize()
	{
		for(usize i = 0; i < m_swapchain->ImageCount(); i++)
		{
			m_frameFences[i]->WaitIdle();
			m_sceneFinishedSems[i]->WaitIdle();
		}

		m_device->WaitIdle();
	}

	void GraphicsSystem::AdvanceFrame()
	{
		m_currentFrame = (m_currentFrame + 1) % m_swapchain->ImageCount();
	}
}
