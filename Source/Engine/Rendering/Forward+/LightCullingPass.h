#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class LightCullingPass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(LightCullingPass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph) final;
		void Render(const Engine::RenderDrawContext& context) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Compute; }
	};
}