#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Shader/ShaderModule.h>
#include <Runtime/RHI/Pipeline/GraphicsPipeline.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class ForwardPass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(ForwardPass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph) final;
		void Render(const Engine::RenderDrawContext& context) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Graphics; }

	private:
		UniquePtr<RHI::ShaderModule> m_vertexShader = nullptr;
		UniquePtr<RHI::ShaderModule> m_fragmentShader = nullptr;

		UniquePtr<RHI::PipelineLayout> m_layout = nullptr;
		UniquePtr<RHI::GraphicsPipeline> m_pipeline = nullptr;

		UniquePtr<RHI::Buffer> m_drawIdBuffer = nullptr;
		UniquePtr<RHI::Buffer> m_drawDataBuffer = nullptr;
		UniquePtr<RHI::Buffer> m_indirectBuffer = nullptr;

		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		u32 m_drawDataView = kInvalid32;
	};
}