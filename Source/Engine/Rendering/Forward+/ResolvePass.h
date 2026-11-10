#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class ResolvePass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(ResolvePass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph) final;
		void Render(const Engine::Blackboard& board) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Graphics; }
	};
}