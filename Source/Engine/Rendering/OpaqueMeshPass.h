#pragma once

#include <Engine/Rendering/RenderGraph/RenderPass.h>
#include <Engine/Rendering/RenderGraph/RenderOrderAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(RenderOrder[0]);
	class ENGINE_API OpaqueMeshPass : public RenderPass 
	{
		HORIZON_TYPE_REFLECT(OpaqueMeshPass);

		struct OpaqueMeshConstants
		{
			Math::Mat4f worldMatrix;
			u32 frameDataIndex;
			u32 vertexBufferIndex;
			u32 firstVertex;
		};
	public:
		void OnInitialize() final;
		void OnFinalize() final;

		void RenderFrame(const RenderPassFrame& frame) final;

	private:
		RHI::GfxPipeline* m_opaquePipeline = nullptr;
	};
}