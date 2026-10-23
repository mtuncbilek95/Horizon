#pragma once

#include <Engine/Rendering/RenderGraph/RenderableObject.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Texture/GfxTexture.h>

namespace Horizon::Engine
{
	struct RenderPassFrame
	{
		RHI::GfxCommandList* pCmdList = nullptr;
		EngineFrame* pEngineFrame = nullptr;
		u32 frameIndex = kInvalid32;
		u32 frameDataIndex = kInvalid32;
		u32 vertexBufferIndex = kInvalid32;

		RHI::GfxTexture* pColorTarget = nullptr;
		RHI::GfxTexture* pDepthTarget = nullptr;
		RHI::GfxBuffer* pIndexBuffer = nullptr;
		Math::Vec2u targetSize = { 0, 0 };

		Math::Mat4f viewMatrix;
		Math::Mat4f projMatrix;
		Math::Mat4f viewProjMatrix;
		Math::Vec3f cameraPosition;

		List<RenderableObject> graphReadyObjects;
	};
}