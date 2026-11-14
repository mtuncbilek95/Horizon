#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Sampler/Sampler.h>
#include <Runtime/RHI/Shader/ShaderModule.h>
#include <Runtime/RHI/Pipeline/GraphicsPipeline.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class FXAAPass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(FXAAPass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph) final;
		void Render(const Engine::Blackboard& board) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Graphics; }

	private:
		UniquePtr<RHI::ShaderModule> m_vertexShader = nullptr;
		UniquePtr<RHI::ShaderModule> m_fragmentShader = nullptr;

		UniquePtr<RHI::GraphicsPipeline> m_pipeline = nullptr;
		UniquePtr<RHI::Sampler> m_sampler = nullptr;

		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		RHI::DescriptorHeap* m_samplerHeap = nullptr;
		u32 m_sourceView = kInvalid32;
		u32 m_samplerView = kInvalid32;
	};
}
