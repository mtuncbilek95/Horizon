#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Shader/ShaderModule.h>
#include <Runtime/RHI/Pipeline/ComputePipeline.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Engine
{
	class GraphicsContext;
}

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class CullClusterPass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(CullClusterPass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph) final;
		void Render(const Engine::Blackboard& board) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Compute; }

	private:
		Engine::GraphicsContext* m_graphics = nullptr;

		UniquePtr<RHI::ShaderModule> m_clusterMarkShader = nullptr;
		UniquePtr<RHI::ComputePipeline> m_cullPipeline = nullptr;

		UniquePtr<RHI::ShaderModule> m_cullShader = nullptr;
		UniquePtr<RHI::ComputePipeline> m_markPipeline = nullptr;

		UniquePtr<RHI::Buffer> m_pointLightBuffer = nullptr;

		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		u32 m_clusterView = kInvalid32;
		u32 m_pointLightView = kInvalid32;
		u32 m_lightGridView = kInvalid32;
		u32 m_activeClusterView = kInvalid32;
		u32 m_depthView = kInvalid32;
	};
}