#pragma once

#include <Engine/Rendering/RenderGraph/RenderableObject.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

namespace Horizon::Engine
{
	struct RenderPassFrame
	{
		RHI::CommandList* pCmdList = nullptr;
		EngineFrame* pEngineFrame = nullptr;
		u32 frameIndex = kInvalid32;
		u32 frameDataIndex = kInvalid32;
		u32 vertexBufferIndex = kInvalid32;

		RHI::Texture* pColorTarget = nullptr;
		RHI::TextureView* pColorView = nullptr;
		RHI::Texture* pDepthTarget = nullptr;
		RHI::TextureView* pDepthView = nullptr;
		RHI::Buffer* pIndexBuffer = nullptr;
		Math::Vec2u targetSize = { 0, 0 };

		Math::Mat4f viewMatrix;
		Math::Mat4f projMatrix;
		Math::Mat4f viewProjMatrix;
		Math::Vec3f cameraPosition;

		List<RenderableObject> graphReadyObjects;
	};
}
