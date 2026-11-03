#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class ENGINE_API DepthPrePass : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(DepthPrePass);
	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(Engine::RenderDependency& graph) final;
		void Render() final;
	};
}