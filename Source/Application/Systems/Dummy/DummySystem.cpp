#include "DummySystem.h"

#include <Runtime/Data/DataReader.h>
#include <Runtime/ShaderCompiler/ShaderCompiler.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Shader/GfxShader.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>

#include <Application/Systems/Renderer/RenderSystem.h>

#include <Runtime/MeshLoader/MeshLoader.h>

namespace Horizon
{
	EngineReport DummySystem::OnInitialize()
	{
		m_RS = &RequestSystem<RenderSystem>();
		auto& device = m_RS->Device();

		auto vertSpirv = ShaderCompiler::GenerateSpirv(DataReader::ReadToTextFile("Shaders/triangle.vert"), "main", ShaderStage::Vertex);
		auto fragSpirv = ShaderCompiler::GenerateSpirv(DataReader::ReadToTextFile("Shaders/triangle.frag"), "main", ShaderStage::Fragment);

		m_vertShader = device.CreateShader(GfxShaderDesc()
			.setStage(ShaderStage::Vertex)
			.setByteCode(*vertSpirv));

		m_fragShader = device.CreateShader(GfxShaderDesc()
			.setStage(ShaderStage::Fragment)
			.setByteCode(*fragSpirv));

		m_dummyTestPipeline = device.CreateGraphicsPipeline(GfxGraphicsPipelineDesc()
			.addShader(m_vertShader.get())
			.addShader(m_fragShader.get())
			.setInput(InputAssembler()
				.setTopology(PrimitiveTopology::TriangleList))
			.setRasterizer(RasterizerState()
				.setPolygonMode(PolygonMode::Fill)
				.setCullMode(CullMode::Back)
				.setFaceOrientation(FaceOrientation::CW))
			.setBlend(BlendState()
				.addAttachment(BlendAttachment()
					.setBlendEnable(false)
					.setColorMask(ColorComponent::All)))
			.setDepthStencil(DepthStencilState()
				.setDepthTestEnable(false)
				.setDepthWriteEnable(false)
				.setDepthOp(CompareOp::Less))
			.setViewport(Viewport()
				.setDepth({ 0.f, 1.f })
				.setSize({ 1920.f, 1080.f })
				.setPosition({ 0.f, 0.f }))
			.setScissor(Scissor()
				.setExtent({ 1920, 1080 })
				.setOffset({ 0, 0 }))
			.setDynamicRendering(DynamicRendering()
				.addColorAttachmentFormat(ImageFormat::R8G8B8A8_UNorm)
			));

		m_batchPool = device.CreateCommandPool(GfxCommandPoolDesc()
			.setQueue(&m_RS->GraphicsQueue())
			.setFlags(CommandPoolFlags::ResetCommandBuffer));

		ObjectInfo info = MeshLoader::LoadFromGltf("Resources/ScifiHelmet/SciFiHelmet.gltf");
		ObjectData data = MeshLoader::ExtractFromGltf(info);

		return EngineReport();
	}

	void DummySystem::OnSync()
	{
		auto tempCmd = m_batchPool->CreateBuffer(CommandLevel::Secondary);

		tempCmd->BeginRecord(DynamicInheritanceDesc()
			.addColorFormat(ImageFormat::R8G8B8A8_UNorm)
			.setDepthFormat(ImageFormat::Undefined)
			, CommandBufferUsage::RenderPassContinue);

		tempCmd->BindPipeline(m_dummyTestPipeline.get());
		tempCmd->DrawVertexed(3, 0, 0, 1);

		tempCmd->EndRecord();

		m_RS->GetBatch().Submit(tempCmd, 0);
	}

	void DummySystem::OnFinalize()
	{
		m_batchPool.reset();
		m_dummyTestPipeline.reset();
		m_fragShader.reset();
		m_vertShader.reset();
	}
}