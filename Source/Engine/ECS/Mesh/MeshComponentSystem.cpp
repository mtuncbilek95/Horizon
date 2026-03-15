#include "MeshComponentSystem.h"

#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/RHI/Shader/GfxShader.h>

#include <Runtime/ShaderCompiler/ShaderCompiler.h>

#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/ECS/Mesh/MeshComponent.h>

namespace Horizon
{
	struct Vertex {
		Math::Vec4f position;
		Math::Vec4f normal;
		Math::Vec4f tangent;
		Math::Vec2f uv;
		Math::Vec2f padding; // Padding to ensure 16-byte alignment
	};

	struct MeshletInfo
	{
		u32 vertexOffset;
		u32 vertexCount;
		u32 indexOffset;
		u32 indexCount;
	};

	std::shared_ptr<GfxBuffer> m_cubeVertexBuffer;
	std::shared_ptr<GfxBuffer> m_cubeIndexBuffer;
	std::shared_ptr<GfxBuffer> m_meshletBuffer;
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

	std::vector<uint32_t> cubeIndices =
	{
		0,  1,  2,  2,  3,  0,
		4,  5,  6,  6,  7,  4,
		8,  9,  10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	MeshletInfo meshletInfo = { 0, 24, 0, 36 };

	SystemReport MeshComponentSystem::OnStart()
	{
		auto& graphicsSystem = RequestSystem<GraphicsSystem>();

		auto* gDevice = graphicsSystem.Device();

		m_meshletBuffer = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(sizeof(MeshletInfo))
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_meshletBuffer->Update(&meshletInfo, sizeof(MeshletInfo));

		m_cubeVertexBuffer = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(cubeVertices.size() * sizeof(Vertex))
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_cubeVertexBuffer->Update(cubeVertices.data(), cubeVertices.size() * sizeof(Vertex));

		m_cubeIndexBuffer = gDevice->CreateBuffer(
			GfxBufferDesc()
			.setMemoryUsage(MemoryUsage::Auto)
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setSize(cubeIndices.size() * sizeof(uint32_t))
			.setUsage(BufferUsage::Storage | BufferUsage::ShaderDeviceAddress)
		);
		m_cubeIndexBuffer->Update(cubeIndices.data(), cubeIndices.size() * sizeof(uint32_t));

		m_meshDescriptorLayout = gDevice->CreateDescriptorLayout(
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
			.addBinding(DescriptorBinding()
				.setBinding(2)
				.setType(DescriptorType::Storage)
				.setStage(ShaderStage::Mesh)
				.setCount(1))
			.setFlags(DescriptorLayoutFlags::Buffer)
		);

		m_meshDescriptorBuffer = gDevice->CreateDescriptorBuffer(
			GfxDescriptorBufferDesc()
			.setAllocationFlags(MemoryAllocation::Mapped | MemoryAllocation::SequentialWrite)
			.setMemoryUsage(MemoryUsage::AutoPreferDevice)
			.setSize(m_meshDescriptorLayout->SizeInBytes())
			.setType(DescriptorBufferType::Resource)
			.setUsage(DescriptorType::Storage)
		);
		m_meshDescriptorBuffer->RequestPayload(m_meshletBuffer.get(), m_meshDescriptorLayout->BindingOffset(0));
		m_meshDescriptorBuffer->RequestPayload(m_cubeVertexBuffer.get(), m_meshDescriptorLayout->BindingOffset(1));
		m_meshDescriptorBuffer->RequestPayload(m_cubeIndexBuffer.get(), m_meshDescriptorLayout->BindingOffset(2));

		std::string meshCode = DataReader::ReadToTextFile("../shaders/Meshlet.mesh");
		std::string fragCode = DataReader::ReadToTextFile("../shaders/Meshlet.frag");

		ReadArray<u32> meshSpirv = ShaderCompiler::GenerateSpirv(meshCode, "main", ShaderStage::Mesh);
		ReadArray<u32> fragSpirv = ShaderCompiler::GenerateSpirv(fragCode, "main", ShaderStage::Fragment);

		m_meshShader = gDevice->CreateShader(
			GfxShaderDesc()
			.setStage(ShaderStage::Mesh)
			.setByteCode(*meshSpirv)
		);
		m_fragShader = gDevice->CreateShader(
			GfxShaderDesc()
			.setStage(ShaderStage::Fragment)
			.setByteCode(*fragSpirv)
		);

		m_meshPipeline = gDevice->CreateGraphicsPipeline(
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
					.setSizeOfData(128u)
					.setStage(ShaderStage::Mesh)
					.setOffset(0)))
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
			.setRenderPass(graphicsSystem.PresentationPass())
			.setFlags(PipelineFlags::DescriptorBuffer));

		return SystemReport();
	}

	void MeshComponentSystem::OnTick()
	{
		auto& graphicsSystem = RequestSystem<GraphicsSystem>();

		graphicsSystem.Submit({ [](GfxCommandBuffer* cmd)
			{
				cmd->BindPipeline(m_meshPipeline.get());
				cmd->BindDescriptorBuffer(m_meshPipeline.get(), ShaderStage::Mesh, 0, m_meshDescriptorBuffer.get());
				cmd->DrawMeshTask(10000, 1, 1);
			} });
	}

	void MeshComponentSystem::OnStop()
	{
	}
}