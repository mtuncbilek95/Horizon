#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Shader/ShaderModule.h>
#include <Runtime/RHI/Pipeline/ComputePipeline.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class ClusterBuildPass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(ClusterBuildPass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph) final;
		void Render(const Engine::Blackboard& board) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Compute; }

	private:
		UniquePtr<RHI::ShaderModule> m_computeShader = nullptr;
		UniquePtr<RHI::ComputePipeline> m_pipeline = nullptr;

		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		u32 m_clusterView = kInvalid32;
	};
}