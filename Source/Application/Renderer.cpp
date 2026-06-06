#include "Renderer.h"

#include <Engine/Core/DataReader.h>
#include <Engine/FrameGraph/FrameGraph.h>
#include <Engine/Asset/AssetRegistry.h>

#include <Engine/ECS/ECS.h>
#include <Engine/ECS/Components/CameraComp.h>
#include <Engine/ECS/Components/MeshComp.h>
#include <Engine/ECS/Components/TransformComp.h>

#include <Engine/Graphics/RHI/GfxPipeline.h>
#include <Engine/Graphics/RHI/GfxTexture.h>
#include <Engine/Graphics/RHI/GfxBuffer.h>
#include <Engine/Graphics/RHI/GfxCommandList.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Horizon
{
	struct ShadowData
	{
		FGTextureHandle shadowMap;
	};

	struct GBufferData
	{
		FGTextureHandle albedo;
		FGTextureHandle normal;
		FGTextureHandle material;
		FGTextureHandle geoNormal;
		FGTextureHandle emissive;
		FGTextureHandle depth;
	};

	struct LightData
	{
		FGTextureHandle albedo;
		FGTextureHandle normal;
		FGTextureHandle geoNormal;
		FGTextureHandle material;
		FGTextureHandle emissive;
		FGTextureHandle depth;
		FGTextureHandle shadow;
		FGTextureHandle hdr;
	};

	struct SkyData
	{
		FGTextureHandle hdr;
		FGTextureHandle depth;
	};

	struct BloomData
	{
		FGTextureHandle src;
		FGTextureHandle tex;
	};

	struct FinalData
	{
		FGTextureHandle hdr;
		FGTextureHandle bloom;
	};

	static GfxTextureDesc ColorDesc(u32 width, u32 height, GfxTextureFormat format, const char* name)
	{
		GfxTextureDesc desc = {};
		desc.width = width; desc.height = height;
		desc.format = format;
		desc.usage = GfxTextureUsage::RenderTarget | GfxTextureUsage::Sampled;
		desc.debugName = name;
		return desc;
	}

	static GfxTextureDesc DepthDesc(u32 width, u32 height, const char* name)
	{
		GfxTextureDesc desc = {};
		desc.width = width; desc.height = height;
		desc.format = GfxTextureFormat::D32;
		desc.usage = GfxTextureUsage::DepthStencil | GfxTextureUsage::Sampled;
		desc.clearDepth = 0.0f;
		desc.debugName = name;
		return desc;
	}

	glm::mat4 OrthoReverseZ(f32 halfWidth, f32 halfHeight, f32 nearZ, f32 farZ)
	{
		glm::mat4 m(1.0f);
		m[0][0] = 1.0f / halfWidth;
		m[1][1] = 1.0f / halfHeight;
		m[2][2] = 1.0f / (farZ - nearZ);
		m[3][2] = farZ / (farZ - nearZ);
		return m;
	}

	static GfxPipelineHandle BuildPipeline(GfxGraphicsPipelineDesc desc, const char* vsPath, const char* psPath)
	{
		auto vertexShader = DataReader::LoadFile(vsPath);
		desc.vertex = { vertexShader.data(), vertexShader.size() };

		if (psPath)
		{
			auto pixelShader = DataReader::LoadFile(psPath);
			desc.pixel = { pixelShader.data(), pixelShader.size() };
			return GfxDevice::CreatePipeline(desc);
		}
		return GfxDevice::CreatePipeline(desc);
	}

	void Renderer::Initialize(u32 framesInFlight)
	{
		m_framesInFlight = framesInFlight;

		GfxBufferDesc frameBufferDesc = {};
		frameBufferDesc.size = (u64)m_framesInFlight * sizeof(FrameConstants);
		frameBufferDesc.stride = sizeof(FrameConstants);
		frameBufferDesc.usage = GfxBufferUsage::Storage;
		frameBufferDesc.memory = GfxMemoryType::Upload;
		m_frameBuffer = GfxDevice::CreateBuffer(frameBufferDesc);
		m_frameBufferShaderView = GfxDevice::BindlessShaderView(m_frameBuffer);

		CreatePipelines();
	}

	void Renderer::CreatePipelines()
	{
		{ // Shadow Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = true;
			desc.depthWrite = true;
			desc.depthCompare = GfxCompareOp::GreaterEqual;
			desc.colorCount = 0;
			desc.depthFormat = GfxTextureFormat::D32;
			desc.debugName = "Shadow Pass";
			m_shadowPipeline = BuildPipeline(desc, "Shaders/ShadowPass.vert", nullptr);
		}

		{ // GBuffer Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = true;
			desc.depthWrite = true;
			desc.depthCompare = GfxCompareOp::GreaterEqual;
			desc.colorCount = 5;
			desc.colorFormats[0] = GfxTextureFormat::RGBA8;
			desc.colorFormats[1] = GfxTextureFormat::RGBA16F;
			desc.colorFormats[2] = GfxTextureFormat::RGBA8;
			desc.colorFormats[3] = GfxTextureFormat::RGBA16F;
			desc.colorFormats[4] = GfxTextureFormat::R11G11B10F;
			desc.depthFormat = GfxTextureFormat::D32;
			desc.debugName = "GBuffer";
			m_gbufferPipeline = BuildPipeline(desc, "Shaders/GBuffer.vert", "Shaders/GBuffer.frag");
		}

		{ // Light Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = false;
			desc.depthWrite = false;
			desc.colorCount = 1;
			desc.colorFormats[0] = GfxTextureFormat::RGBA16F;
			desc.depthFormat = GfxTextureFormat::Undefined;
			desc.debugName = "LightPass";
			m_lightPipeline = BuildPipeline(desc, "Shaders/LightPass.vert", "Shaders/LightPass.frag");
		}

		{ // Procedural Skybox Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = false;
			desc.depthWrite = false;
			desc.colorCount = 1;
			desc.colorFormats[0] = GfxTextureFormat::RGBA16F;
			desc.depthFormat = GfxTextureFormat::Undefined;
			desc.debugName = "ProceduralSky";
			m_skyPipeline = BuildPipeline(desc, "Shaders/ProceduralSkyPass.vert", "Shaders/ProceduralSkyPass.frag");
		}

		{ // Bloom Extract Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = false; 
			desc.depthWrite = false;
			desc.colorCount = 1;
			desc.colorFormats[0] = GfxTextureFormat::RGBA16F;
			desc.depthFormat = GfxTextureFormat::Undefined;
			desc.debugName = "BloomExtract";
			m_bloomExtractPipeline = BuildPipeline(desc, "Shaders/FinalPass.vert", "Shaders/BloomExtract.frag");
		}

		{ // Bloom Blur Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = false; desc.depthWrite = false;
			desc.colorCount = 1;
			desc.colorFormats[0] = GfxTextureFormat::RGBA16F;
			desc.depthFormat = GfxTextureFormat::Undefined;
			desc.debugName = "BloomBlur";
			m_bloomBlurPipeline = BuildPipeline(desc, "Shaders/FinalPass.vert", "Shaders/BloomBlur.frag");
		}

		{ // Final Pass Pipeline
			GfxGraphicsPipelineDesc desc = {};
			desc.cull = GfxCullMode::None;
			desc.depthTest = false;
			desc.depthWrite = false;
			desc.colorCount = 1;
			desc.colorFormats[0] = GfxTextureFormat::RGBA8;
			desc.depthFormat = GfxTextureFormat::Undefined;
			desc.debugName = "FinalPass";
			m_finalPipeline = BuildPipeline(desc, "Shaders/FinalPass.vert", "Shaders/FinalPass.frag");
		}
	}

	void Renderer::Render(const CameraComp& camera, GfxTextureHandle backBuffer,
		u32 width, u32 height, u32 frameSlot)
	{
		glm::mat4 viewProj = camera.GetProjection() * camera.GetView();

		glm::vec3 lightUp = (fabs(m_lightDirection.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
		glm::mat4 lightView = glm::lookAt(-m_lightDirection * 30.0f, glm::vec3(0.0f), lightUp);
		glm::mat4 lightProj = OrthoReverseZ(25.0f, 25.0f, 0.1f, 80.0f);

		FrameConstants frameConstants = {};
		frameConstants.viewProj = viewProj;
		frameConstants.invViewProj = glm::inverse(viewProj);
		frameConstants.lightViewProj = lightProj * lightView;
		frameConstants.lightDirection = glm::vec4(m_lightDirection, 0.0f);
		frameConstants.lightColor = m_lightColor;
		frameConstants.camPos = camera.ComputePosition();

		GfxDevice::WriteBuffer(m_frameBuffer, &frameConstants, sizeof(frameConstants),
			(u64)frameSlot * sizeof(FrameConstants));

		MainGraph().Reset();
		BuildFrameGraph(backBuffer, width, height, frameSlot);
		MainGraph().Compile();
		MainGraph().Execute();
	}

	void Renderer::BuildFrameGraph(GfxTextureHandle backBuffer, u32 width, u32 height, u32 frameSlot)
	{
		// Import swapchain backbuffer to frame graph
		FGTextureHandle backBufferPassHandle = MainGraph().ImportTexture(backBuffer, GfxResourceState::Present, "Backbuffer", width, height);

		// Create shadow pass
		const ShadowData& shadow = MainGraph().AddRasterPass<ShadowData>("ShadowPass",
			[=](FrameGraphBuilder& builder, ShadowData& data) // Setup
			{
				FGTextureHandle shadowMap = builder.CreateTexture(DepthDesc(ShadowMapSize, ShadowMapSize, "Shadow Map"), "Shadow Map");
				data.shadowMap = builder.SetDepthAttachment(shadowMap);
			},
			[this, frameSlot](const ShadowData&, FrameGraphResources&, GfxCommandList* cmdList) // Execute
			{
				GfxCmdList::BindPipeline(cmdList, m_shadowPipeline);
				MainWorld().ForEach<MeshComp>([&](EntityHandle entity, MeshComp& meshComp)
					{
						const MeshAsset& mesh = AssetSystem().GetMesh(meshComp.meshId);
						ShadowPush push = {};
						push.model = MainWorld().GetComponent<TransformComp>(entity).GetMatrix();
						push.vertexBufferIndex = mesh.vertexShaderView;
						push.frameBufferIndex = m_frameBufferShaderView;
						push.frameSlot = frameSlot;
						GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
						GfxCmdList::BindIndexBuffer(cmdList, mesh.indexBuffer, GfxIndexType::UInt32);
						GfxCmdList::DrawIndexed(cmdList, mesh.indexCount, 1);
					});
			});

		// Create gbuffer pass
		const GBufferData& gbuffer = MainGraph().AddRasterPass<GBufferData>("GBuffer",
			[=](FrameGraphBuilder& builder, GBufferData& data) // Setup
			{
				FGTextureHandle albedo = builder.CreateTexture(ColorDesc(width, height, GfxTextureFormat::RGBA8, "GAlbedo"), "GAlbedo");
				FGTextureHandle normal = builder.CreateTexture(ColorDesc(width, height, GfxTextureFormat::RGBA16F, "GNormal"), "GNormal");
				FGTextureHandle material = builder.CreateTexture(ColorDesc(width, height, GfxTextureFormat::RGBA8, "GMaterial"), "GMaterial");
				FGTextureHandle geoNormal = builder.CreateTexture(ColorDesc(width, height, GfxTextureFormat::RGBA16F, "GGeoNormal"), "GGeoNormal");
				FGTextureHandle emissive = builder.CreateTexture(ColorDesc(width, height, GfxTextureFormat::R11G11B10F, "GEmissive"), "GEmissive");
				FGTextureHandle depth = builder.CreateTexture(DepthDesc(width, height, "Scene Depth"), "Scene Depth");

				data.albedo = builder.SetColorAttachment(0, albedo);
				data.normal = builder.SetColorAttachment(1, normal);
				data.material = builder.SetColorAttachment(2, material);
				data.geoNormal = builder.SetColorAttachment(3, geoNormal);
				data.emissive = builder.SetColorAttachment(4, emissive);
				data.depth = builder.SetDepthAttachment(depth);
			},
			[this, frameSlot](const GBufferData&, FrameGraphResources&, GfxCommandList* cmdList) // Execute
			{
				GfxCmdList::BindPipeline(cmdList, m_gbufferPipeline);
				MainWorld().ForEach<MeshComp>([&](EntityHandle entity, MeshComp& meshComp)
					{
						const MeshAsset& mesh = AssetSystem().GetMesh(meshComp.meshId);
						GBufferPush push = {};
						push.model = MainWorld().GetComponent<TransformComp>(entity).GetMatrix();
						push.vertexBufferIndex = mesh.vertexShaderView;
						push.materialBufferIndex = AssetSystem().MaterialBufferIndex();
						push.materialIndex = meshComp.materialId.index();
						push.frameBufferIndex = m_frameBufferShaderView;
						push.frameSlot = frameSlot;
						GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
						GfxCmdList::BindIndexBuffer(cmdList, mesh.indexBuffer, GfxIndexType::UInt32);
						GfxCmdList::DrawIndexed(cmdList, mesh.indexCount, 1);
					});
			});

		// Create light pass
		const LightData& light = MainGraph().AddRasterPass<LightData>("LightPass",
			[=](FrameGraphBuilder& builder, LightData& data) // Setup
			{
				data.albedo = builder.Read(gbuffer.albedo, GfxResourceState::ShaderResource);
				data.normal = builder.Read(gbuffer.normal, GfxResourceState::ShaderResource);
				data.geoNormal = builder.Read(gbuffer.geoNormal, GfxResourceState::ShaderResource);
				data.material = builder.Read(gbuffer.material, GfxResourceState::ShaderResource);
				data.emissive = builder.Read(gbuffer.emissive, GfxResourceState::ShaderResource);
				data.depth = builder.Read(gbuffer.depth, GfxResourceState::ShaderResource);
				data.shadow = builder.Read(shadow.shadowMap, GfxResourceState::ShaderResource);

				FGTextureHandle hdr = builder.CreateTexture(ColorDesc(width, height, GfxTextureFormat::RGBA16F, "HDR"), "HDR");
				data.hdr = builder.SetColorAttachment(0, hdr);
			},
			[this, frameSlot](const LightData& data, FrameGraphResources& resources, GfxCommandList* cmdList) // Execute
			{
				LightPush push = {};
				push.albedoIndex = resources.GetTextureShaderView(data.albedo);
				push.normalIndex = resources.GetTextureShaderView(data.normal);
				push.geoNormalIndex = resources.GetTextureShaderView(data.geoNormal);
				push.materialIndex = resources.GetTextureShaderView(data.material);
				push.emissiveIndex = resources.GetTextureShaderView(data.emissive);
				push.depthIndex = resources.GetTextureShaderView(data.depth);
				push.shadowIndex = resources.GetTextureShaderView(data.shadow);
				push.frameBufferIndex = m_frameBufferShaderView;
				push.frameSlot = frameSlot;

				GfxCmdList::BindPipeline(cmdList, m_lightPipeline);
				GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
				GfxCmdList::Draw(cmdList, 3, 1);
			});

		// Create skybox pass
		const SkyData& sky = MainGraph().AddRasterPass<SkyData>("ProceduralSky",
			[=](FrameGraphBuilder& builder, SkyData& data) // Setup
			{
				data.depth = builder.Read(gbuffer.depth, GfxResourceState::ShaderResource);
				data.hdr = builder.SetColorAttachment(0, light.hdr, GfxLoadOp::Load, GfxStoreOp::Store);
			},
			[this, frameSlot](const SkyData& data, FrameGraphResources& resources, GfxCommandList* cmdList) // Execute
			{
				SkyPush push = {};
				push.depthIndex = resources.GetTextureShaderView(data.depth);
				push.frameBufferIndex = m_frameBufferShaderView;
				push.frameSlot = frameSlot;

				GfxCmdList::BindPipeline(cmdList, m_skyPipeline);
				GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
				GfxCmdList::Draw(cmdList, 3, 1);
			});

		u32 bloomWidth = width / 2;
		u32 bloomHeight = height / 2;

		// Create blur extraction pass
		const BloomData& extract = MainGraph().AddRasterPass<BloomData>("BloomExtract",
			[=](FrameGraphBuilder& builder, BloomData& data)
			{
				data.src = builder.Read(sky.hdr, GfxResourceState::ShaderResource);
				data.tex = builder.SetColorAttachment(0,
					builder.CreateTexture(ColorDesc(bloomWidth, bloomHeight, GfxTextureFormat::RGBA16F, "BloomExtract"), "BloomExtract"));
			},
			[this](const BloomData& data, FrameGraphResources& resources, GfxCommandList* cmdList)
			{
				BloomExtractPush push = {};
				push.hdrIndex = resources.GetTextureShaderView(data.src);
				push.threshold = m_bloomThreshold;
				GfxCmdList::BindPipeline(cmdList, m_bloomExtractPipeline);
				GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
				GfxCmdList::Draw(cmdList, 3, 1);
			});

		// Create blur horizontal pass
		const BloomData& blurH = MainGraph().AddRasterPass<BloomData>("BloomBlurH",
			[=](FrameGraphBuilder& builder, BloomData& data)
			{
				data.src = builder.Read(extract.tex, GfxResourceState::ShaderResource);
				data.tex = builder.SetColorAttachment(0,
					builder.CreateTexture(ColorDesc(bloomWidth, bloomHeight, GfxTextureFormat::RGBA16F, "BloomBlurH"), "BloomBlurH"));
			},
			[this](const BloomData& data, FrameGraphResources& resources, GfxCommandList* cmdList)
			{
				BloomBlurPush push = {};
				push.srcIndex = resources.GetTextureShaderView(data.src);
				push.horizontal = 1;
				GfxCmdList::BindPipeline(cmdList, m_bloomBlurPipeline);
				GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
				GfxCmdList::Draw(cmdList, 3, 1);
			});

		// Create blur vertical pass
		const BloomData& blurV = MainGraph().AddRasterPass<BloomData>("BloomBlurV",
			[=](FrameGraphBuilder& builder, BloomData& data)
			{
				data.src = builder.Read(blurH.tex, GfxResourceState::ShaderResource);
				data.tex = builder.SetColorAttachment(0,
					builder.CreateTexture(ColorDesc(bloomWidth, bloomHeight, GfxTextureFormat::RGBA16F, "BloomBlurV"), "BloomBlurV"));
			},
			[this](const BloomData& data, FrameGraphResources& resources, GfxCommandList* cmdList)
			{
				BloomBlurPush push = {};
				push.srcIndex = resources.GetTextureShaderView(data.src);
				push.horizontal = 0;
				GfxCmdList::BindPipeline(cmdList, m_bloomBlurPipeline);
				GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
				GfxCmdList::Draw(cmdList, 3, 1);
			});

		// Create final pass
		MainGraph().AddRasterPass<FinalData>("FinalPass",
			[=](FrameGraphBuilder& builder, FinalData& data) // Setup
			{
				data.hdr = builder.Read(sky.hdr, GfxResourceState::ShaderResource);
				data.bloom = builder.Read(blurV.tex, GfxResourceState::ShaderResource);
				builder.SetColorAttachment(0, backBufferPassHandle, GfxLoadOp::Clear, GfxStoreOp::Store);
				builder.SetSideEffect();
			},
			[this, frameSlot](const FinalData& data, FrameGraphResources& resources, GfxCommandList* cmdList) // Execute
			{
				FinalPush push = {};
				push.hdrIndex = resources.GetTextureShaderView(data.hdr);
				push.bloomIndex = resources.GetTextureShaderView(data.bloom);
				push.frameBufferIndex = m_frameBufferShaderView;
				push.frameSlot = frameSlot;

				GfxCmdList::BindPipeline(cmdList, m_finalPipeline);
				GfxCmdList::SetConstants(cmdList, &push, sizeof(push));
				GfxCmdList::Draw(cmdList, 3, 1);
			});
	}

	void Renderer::Shutdown() const
	{
		GfxDevice::DestroyPipeline(m_shadowPipeline);
		GfxDevice::DestroyPipeline(m_gbufferPipeline);
		GfxDevice::DestroyPipeline(m_lightPipeline);
		GfxDevice::DestroyPipeline(m_skyPipeline);
		GfxDevice::DestroyPipeline(m_bloomExtractPipeline);
		GfxDevice::DestroyPipeline(m_bloomBlurPipeline);
		GfxDevice::DestroyPipeline(m_finalPipeline);
		GfxDevice::DestroyBuffer(m_frameBuffer);
	}
}