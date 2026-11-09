#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Shader/ShaderModule.h>
#include <Runtime/RHI/Pipeline/GraphicsPipeline.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DynamicUsageAttribute.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Rendering::Pass
{
	HCLASS(DynamicUsage);
	class FXAAPass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(FXAAPass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph) final;
		void Render(const Engine::RenderDrawContext& context) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Graphics; }
	};
}