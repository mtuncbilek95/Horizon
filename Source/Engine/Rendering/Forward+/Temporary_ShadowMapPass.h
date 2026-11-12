#pragma once

#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderableDirectionalLight.h>
#include <Runtime/RHI/Shader/ShaderModule.h>
#include <Runtime/RHI/Pipeline/GraphicsPipeline.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Memory/UniquePtr.h>

namespace Horizon::Rendering::Pass
{
	HCLASS();
	class ShadowMapPass final : public Engine::RenderNode
	{
		HORIZON_TYPE_REFLECT(ShadowMapPass);
	public:
		static constexpr f32 ShadowHalfExtent = 40.0f;
		static constexpr f32 ShadowHalfDepth = 250.0f;

	public:
		static Math::Vec3f ResolveLightDirection(const Engine::RenderableDirectionalLight* pLight);
		static Math::Mat4f BuildLightViewProj(const Engine::RenderableCamera& camera, const Math::Vec3f& lightDirection, u32 mapSize);

	public:
		b8 OnInitialize() final;
		void OnFinalize() final;
		void Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph) final;
		void Render(const Engine::Blackboard& board) final;

		RHI::QueueType GetRunType() const final { return RHI::QueueType::Graphics; }

	private:
		UniquePtr<RHI::ShaderModule> m_vertexShader = nullptr;
		UniquePtr<RHI::GraphicsPipeline> m_pipeline = nullptr;
	};
}
